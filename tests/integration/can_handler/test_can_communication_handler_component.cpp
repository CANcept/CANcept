#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
using namespace CanHandler;

class CanCommunicationHandlerIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canCommunicationHandler = std::make_unique<CanCommunicationHandler>(*eventBroker);
        deviceManager = std::make_unique<TestHelpers::SocketCanDeviceManager>("vcan0");
        receiveConnectionRaw = eventBroker->subscribe<Core::ReceivedCanRawEvent>(
            [this](const Core::ReceivedCanRawEvent& event) {
                lastReceivedMessageRaw = std::make_unique<Core::RawCanMessage>(event.canMessage);
                receiveMessageCounterRaw++;
            });
        receiveConnectionDbc = eventBroker->subscribe<Core::ReceivedCanDbcEvent>(
            [this](const Core::ReceivedCanDbcEvent& event) {
                lastReceivedMessageDbc = std::make_unique<Core::DbcCanMessage>(event.canMessage);
                receiveMessageCounterDbc++;
            });
        try
        {
            deviceManager->create();
            deviceManager->up();
            canDeviceCreated = true;
        } catch (const std::exception& e)  // not root user or could not add device
        {
            return;
        }
        eventBroker->publish(Core::AppStartedEvent{});
        isStarted = true;
    }

    void TearDown() override
    {
        receiveConnectionDbc.release();
        receiveConnectionRaw.release();

        if (isStarted)
        {
            eventBroker->publish(Core::AppStoppedEvent{});
            isStarted = false;
        }
        if (canDeviceCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }

        canCommunicationHandler.reset();
        eventBroker.reset();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<CanCommunicationHandler> canCommunicationHandler;
    Core::Connection receiveConnectionRaw;
    std::unique_ptr<Core::RawCanMessage> lastReceivedMessageRaw;
    int receiveMessageCounterRaw = 0;
    Core::Connection receiveConnectionDbc;
    std::unique_ptr<Core::DbcCanMessage> lastReceivedMessageDbc;
    int receiveMessageCounterDbc = 0;
    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    bool isStarted = false;
    bool canDeviceCreated = false;
};

TEST_F(CanCommunicationHandlerIntegrationTest, SendRawAndReceiveMessages)
{
    if (!canDeviceCreated)
    {
        return;
    }
    std::list<Core::SelectOption> options;
    eventBroker->publish(Core::GetAvailableCanDriversEvent(&options));

    EXPECT_FALSE(options.empty());
    EXPECT_EQ(options.front().value, "vcan0");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    eventBroker->publish(Core::CanDriverChangeEvent("vcan0"));
    ASSERT_TRUE(canDeviceCreated);
    ASSERT_TRUE(isStarted);
    ASSERT_EQ(receiveMessageCounterRaw, 0);
    ASSERT_EQ(receiveMessageCounterDbc, 0);
    Core::RawCanMessage message;
    message.data = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    message.messageId = 0x1;
    message.dlc = 8;
    eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcConfigBuilder()
                                 .message(TestHelpers::DbcMessageBuilder(0x1, "TestMessage")
                                              .signal(TestHelpers::DbcSignalBuilder("Signal1")
                                                          .startBit(0)
                                                          .size(8)
                                                          .littleEndian()
                                                          .unsigned_()
                                                          .factor(1.0)
                                                          .offset(0.0)
                                                          .range(0.0, 255.0)
                                                          .unit("unit1")
                                                          .receiver("ECU1"))
                                              .build())
                                 .build(),
                             ""));
    eventBroker->publish(Core::SendCanMessageRawEvent(message));
    std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    ASSERT_EQ(receiveMessageCounterRaw, 1);
    ASSERT_EQ(receiveMessageCounterDbc, 1);
    ASSERT_EQ(lastReceivedMessageRaw->messageId, message.messageId);
    ASSERT_EQ(lastReceivedMessageDbc->messageId, message.messageId);
}

TEST_F(CanCommunicationHandlerIntegrationTest, SendDbcAndReceiveMessages)
{
    if (!canDeviceCreated)
    {
        return;
    }

    ASSERT_TRUE(canDeviceCreated);
    ASSERT_TRUE(isStarted);
    ASSERT_EQ(receiveMessageCounterRaw, 0);
    ASSERT_EQ(receiveMessageCounterDbc, 0);
    eventBroker->publish(Core::CanDriverChangeEvent("vcan0"));
    Core::DbcCanMessage message;
    message.signalValues = {{"Signal1", 0}};
    message.messageId = 0x1;
    eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcConfigBuilder()
                                 .message(TestHelpers::DbcMessageBuilder(0x1, "TestMessage")
                                              .signal(TestHelpers::DbcSignalBuilder("Signal1")
                                                          .startBit(0)
                                                          .size(8)
                                                          .littleEndian()
                                                          .unsigned_()
                                                          .factor(1.0)
                                                          .offset(0.0)
                                                          .range(0.0, 255.0)
                                                          .unit("unit1")
                                                          .receiver("ECU1"))
                                              .build())
                                 .build(),
                             ""));
    eventBroker->publish(Core::SendCanMessageDbcEvent(message));
    std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    ASSERT_EQ(receiveMessageCounterRaw, 1);
    ASSERT_EQ(receiveMessageCounterDbc, 1);
    ASSERT_EQ(lastReceivedMessageRaw->messageId, message.messageId);
    ASSERT_EQ(lastReceivedMessageDbc->messageId, message.messageId);
}
