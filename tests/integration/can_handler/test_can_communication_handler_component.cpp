/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
        if (!getuid())
        {
            try
            {
                deviceManager->create();
                deviceManager->up();
                canDeviceCreated = true;
            } catch (const std::exception& e)  // not root user or could not add device
            {
            }
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
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    eventBroker->publish(Core::CanDriverChangeEvent("vcan0"));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(message, encoded));
    eventBroker->publish(Core::SendCanMessageRawEvent(encoded));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_EQ(receiveMessageCounterRaw, 1);
    ASSERT_EQ(receiveMessageCounterDbc, 1);
    ASSERT_EQ(lastReceivedMessageRaw->messageId, message.messageId);
    ASSERT_EQ(lastReceivedMessageDbc->messageId, message.messageId);
}
