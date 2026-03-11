#include <gtest/gtest.h>
#include <unistd.h>

#include <QTest>
#include <atomic>
#include <chrono>
#include <thread>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_switch.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/model/logging_model.hpp"
#include "sending/sending_component.hpp"
#include "sending/view/components/repeated_sending_card.hpp"
#include "sending/view/dbc_based_sending_subview.hpp"
#include "sending/view/raw_sending_subview.hpp"
#include "sending/view/sending_view.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

using namespace Logging;

class SendingSystemTest : public ::testing::Test
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
        sending = std::make_unique<Sending::SendingComponent>(*broker);
        loggingModel = std::make_unique<LoggingModel>();

        broker->publish<Core::AppStartedEvent>({});
        QTest::qWait(100);
    }

    void TearDown() override
    {
        if (broker)
        {
            broker->publish<Core::AppStoppedEvent>({});
            QTest::qWait(50);
        }

        sending.reset();
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
        QTest::qWait(200);
    }

    auto getSendingView() const -> Sending::SendingView*
    {
        return qobject_cast<Sending::SendingView*>(sending->getView());
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<Sending::SendingComponent> sending;
    std::unique_ptr<LoggingModel> loggingModel;
    bool vcanCreated = false;
};

TEST_F(SendingSystemTest, FillRawForm_ClickSend_RawEventFiresAndSessionCaptures)
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

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);

    auto* rawView = view->rawSubView();
    rawView->canIdEditor()->setText("042");
    rawView->messageDataEditor()->setText("AB CD");
    QTest::qWait(50);

    rawView->sendButton()->click();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_GT(rawEventCount, 0) << "ReceivedCanRawEvent was never published";
    EXPECT_GT(capturedInSession, 0) << "Active RAW session did not receive the event";
}

TEST_F(SendingSystemTest, EnableRepeatedSending_ClickSend_MultipleRawEventsFireWithinInterval)
{
    connectVcan();

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);

    auto* rawView = view->rawSubView();
    rawView->canIdEditor()->setText("042");
    rawView->messageDataEditor()->setText("AB CD");
    QTest::qWait(50);

    // Enable repeated sending toggle
    auto* repeatedCard = rawView->repeatedSendingCard();
    ASSERT_NE(repeatedCard, nullptr);
    auto* toggleSwitch = repeatedCard->findChild<Core::StyledSwitch*>();
    ASSERT_NE(toggleSwitch, nullptr) << "Toggle switch not found in RepeatedSendingCard";
    toggleSwitch->setChecked(true);
    QTest::qWait(30);
    ASSERT_TRUE(repeatedCard->isRepeatedSendingEnabled())
        << "Repeated sending not enabled after setChecked";

    // Set interval to 200ms
    repeatedCard->frequencyEditor()->setText("200");
    QTest::qWait(30);

    // Count received raw CAN events
    std::atomic<int> rawEventCount{0};
    auto rawConn = broker->subscribe<Core::ReceivedCanRawEvent>(
        [&](const Core::ReceivedCanRawEvent&) { ++rawEventCount; });

    // Click send button — should start repeated sending
    rawView->sendButton()->click();
    QTest::qWait(30);

    // Wait ~1 second: at 200ms interval, expect at least 3 messages
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    QTest::qWait(50);

    const int countAfterRunning = rawEventCount.load();
    EXPECT_GE(countAfterRunning, 3)
        << "Expected at least 3 repeated messages in 1.1 seconds at 200ms interval";

    // Click again to stop
    rawView->sendButton()->click();
    QTest::qWait(50);

    // Wait another second and confirm no more events accumulate
    const int countAfterStop = rawEventCount.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    QTest::qWait(50);

    EXPECT_LE(rawEventCount.load() - countAfterStop, 1)
        << "Events should stop accumulating after repeated sending is stopped";
}

TEST_F(SendingSystemTest, FillDbcForm_ClickSend_DbcEventFiresAndSessionCaptures)
{
    loadDbc(TestHelpers::makeTempDbcFile());

    bool dbcSent = false;
    auto sentConn = broker->subscribe<Core::SendCanMessageDbcEvent>(
        [&](const Core::SendCanMessageDbcEvent&) { dbcSent = true; });

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

    connectVcan();

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);
    view->displayMode(1);
    QTest::qWait(50);

    auto* dbcView = view->dbcSubView();
    const auto cards = dbcView->findChildren<Core::DbcMessageCard*>();
    ASSERT_FALSE(cards.isEmpty()) << "No DBC message cards found after loading DBC";

    cards.first()->headerCheckbox()->setChecked(true);
    QTest::qWait(50);

    ASSERT_TRUE(dbcView->sendButton()->isEnabled())
        << "DBC send button not enabled after selecting signal";

    const std::map<uint32_t, QStringList> selectedSignals = {
        {TestHelpers::kTestMsgId, {TestHelpers::kTestSignalName}}};
    const std::map<uint16_t, std::pair<int, int>> beforeAfter = {
        {static_cast<uint16_t>(TestHelpers::kTestMsgId), {0, 0}}};
    loggingModel->startNewDbcLogSession(selectedSignals, beforeAfter);
    ASSERT_TRUE(loggingModel->isRecording());

    dbcView->sendButton()->click();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_TRUE(dbcSent) << "SendCanMessageDbcEvent was never published";
    EXPECT_GT(dbcEventCount, 0) << "ReceivedCanDbcEvent was never published";
    EXPECT_GT(capturedInSession, 0) << "Active DBC session did not receive the event";
}
