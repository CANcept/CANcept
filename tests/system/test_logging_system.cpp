#include <gtest/gtest.h>
#include <unistd.h>

#include <QSignalSpy>
#include <QTest>
#include <chrono>
#include <thread>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/model/logging_model.hpp"
#include "monitoring/monitoring_component.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

using namespace Logging;

namespace {

auto makeSelectedSignals() -> std::map<uint32_t, QStringList>
{
    return {{TestHelpers::kTestMsgId, {TestHelpers::kTestSignalName}}};
}

auto makeBeforeAfter() -> std::map<uint16_t, std::pair<int, int>>
{
    return {{static_cast<uint16_t>(TestHelpers::kTestMsgId), {0, 0}}};
}

}  // namespace

class CanLoggingSystemTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        if (getuid() != 0)
        {
            GTEST_SKIP() << "System tests require root privileges (run start.sh with sudo)";
        }

        deviceManager = std::make_unique<TestHelpers::SocketCanDeviceManager>("vcan0");
        try
        {
            deviceManager->create();
            deviceManager->up();
            vcanCreated = true;
        } catch (const std::exception& e)
        {
            GTEST_SKIP() << "Failed to set up vcan0: " << e.what();
        }

        broker = std::make_unique<EventBroker::EventBroker>();
        canHandler = std::make_unique<CanHandler::CanCommunicationHandler>(*broker);
        dbcHandler = std::make_unique<CanHandler::DbcHandler>(*broker);
        loggingModel = std::make_unique<LoggingModel>();
        monitoring = std::make_unique<Monitoring::MonitoringComponent>(*broker);

        broker->publish<Core::AppStartedEvent>({});
    }

    void TearDown() override
    {
        if (broker)
        {
            broker->publish<Core::AppStoppedEvent>({});
        }

        monitoring.reset();
        loggingModel.reset();
        dbcHandler.reset();
        canHandler.reset();
        broker.reset();

        if (vcanCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }
    }

    void connectVcan() const
    {
        broker->publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent{"vcan0"});
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    void loadDbc(const std::string& path) const
    {
        broker->publish<Core::ParseDBCRequestEvent>(Core::ParseDBCRequestEvent{path});
        QTest::qWait(100);
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<LoggingModel> loggingModel;
    std::unique_ptr<Monitoring::MonitoringComponent> monitoring;
    bool vcanCreated = false;
};

TEST_F(CanLoggingSystemTest, SendVcan_RawEventFires_AndActiveSessionCapturesIt)
{
    int rawEventCount = 0;
    auto rawConn = broker->subscribe<Core::ReceivedCanRawEvent>(
        [&](const Core::ReceivedCanRawEvent&) { ++rawEventCount; });

    int capturedInSession = 0;
    auto sessionConn =
        broker->subscribe<Core::ReceivedCanRawEvent>([&](const Core::ReceivedCanRawEvent&) {
            if (const auto* session = loggingModel->getSession(loggingModel->getCurrentSessionId());
                session && session->type == RAW && session->isRecording)
            {
                ++capturedInSession;
            }
        });

    connectVcan();

    loggingModel->startNewRawLogsSession();
    ASSERT_TRUE(loggingModel->isRecording());

    Core::RawCanMessage msg;
    msg.messageId = 0x42;
    msg.dlc = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;
    broker->publish<Core::SendCanMessageRawEvent>(Core::SendCanMessageRawEvent{msg});

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_GT(rawEventCount, 0) << "ReceivedCanRawEvent was never published";
    EXPECT_GT(capturedInSession, 0) << "Active RAW session did not receive the event";
}

TEST_F(CanLoggingSystemTest, LoadDbcFile_SendVcan_DbcEventFires_LoggingAndMonitoringCapture)
{
    bool dbcParsed = false;
    auto parsedConn = broker->subscribe<Core::DBCParsedEvent>(
        [&](const Core::DBCParsedEvent&) { dbcParsed = true; });

    loadDbc(TestHelpers::makeTempDbcFile());
    ASSERT_TRUE(dbcParsed) << "DBCParsedEvent not received after ParseDBCRequestEvent";

    int dbcEventCount = 0;
    auto dbcConn = broker->subscribe<Core::ReceivedCanDbcEvent>(
        [&](const Core::ReceivedCanDbcEvent&) { ++dbcEventCount; });

    int capturedInSession = 0;
    auto sessionConn =
        broker->subscribe<Core::ReceivedCanDbcEvent>([&](const Core::ReceivedCanDbcEvent&) {
            if (const auto* session = loggingModel->getSession(loggingModel->getCurrentSessionId());
                session && session->type == DBC_BASED && session->isRecording)
            {
                ++capturedInSession;
            }
        });

    const QSignalSpy monitoringSpy(monitoring.get(),
                                   &Monitoring::MonitoringComponent::dbcFrameReceived);

    connectVcan();

    loggingModel->startNewDbcLogSession(makeSelectedSignals(), makeBeforeAfter());
    ASSERT_TRUE(loggingModel->isRecording());

    Core::RawCanMessage msg;
    msg.messageId = TestHelpers::kTestMsgId;
    msg.dlc = 8;
    msg.data[0] = 0xAB;
    broker->publish<Core::SendCanMessageRawEvent>(Core::SendCanMessageRawEvent{msg});

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_GT(dbcEventCount, 0) << "ReceivedCanDbcEvent was never published";
    EXPECT_GT(capturedInSession, 0) << "Active DBC session did not receive the event";
    EXPECT_GT(monitoringSpy.count(), 0) << "MonitoringComponent::dbcFrameReceived not emitted";
}
