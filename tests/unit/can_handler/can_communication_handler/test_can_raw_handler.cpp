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
#include "tests/helpers/mock_event_broker.hpp"

using namespace CanHandler;

class CanRawHandlerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canRawHandler =
            std::make_unique<CanRawHandler>(*eventBroker, [this](const CanMessage& canMessage) {
                lastSentMessage = std::make_unique<CanMessage>(canMessage);
                sentMessageCounter++;
                return true;
            });
        receiveConnection = eventBroker->subscribe<Core::ReceivedCanRawEvent>(
            [this](const Core::ReceivedCanRawEvent& event) {
                lastReceivedMessage = std::make_unique<Core::RawCanMessage>(event.canMessage);
                receiveMessageCounter++;
            });
    }

    void TearDown() override
    {
        receiveConnection.release();
        canRawHandler.reset();
        eventBroker.reset();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<CanRawHandler> canRawHandler;
    Core::Connection receiveConnection;
    int receiveMessageCounter = 0;
    int sentMessageCounter = 0;
    std::unique_ptr<CanMessage> lastSentMessage;
    std::unique_ptr<Core::RawCanMessage> lastReceivedMessage;
};

TEST_F(CanRawHandlerTest, ConstructsSuccessfuly)
{
    EXPECT_NO_THROW(CanRawHandler(*eventBroker, [](const CanMessage& canMessage) { return true; }));
}

TEST_F(CanRawHandlerTest, SubscribedToOneEvent)
{
    EXPECT_EQ(eventBroker.get()->subscriptionCount<Core::SendCanMessageRawEvent>(), 1);
}

TEST_F(CanRawHandlerTest, ParsesReceivedMessageSuccessfuly)
{
    const CanMessage canMessage{1, {'0', '0', '0', '0', '0', '0', '0', '0'}};
    canRawHandler->parseReceivedMessage(&canMessage);
    EXPECT_NE(lastReceivedMessage.get(), nullptr);
}

TEST_F(CanRawHandlerTest, ParsesReceivedMessageCorrectly)
{
    const CanMessage canMessage{
        1, {'1', '2', '3', '4', '5', '6', '7', '8'}, static_cast<std::chrono::milliseconds>(1000)};
    canRawHandler->parseReceivedMessage(&canMessage);
    EXPECT_NE(lastReceivedMessage.get(), nullptr);
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(lastReceivedMessage->data.at(i), canMessage.getRawFrame().data[i]);
    }
    EXPECT_EQ(lastReceivedMessage->messageId, 1);
    EXPECT_EQ(lastReceivedMessage->receiveTime, static_cast<std::chrono::milliseconds>(1000));
}

TEST_F(CanRawHandlerTest, ReceivedMessageCountCorrect)
{
    const CanMessage canMessage{1, {'0', '0', '0', '0', '0', '0', '0', '0'}};
    canRawHandler->parseReceivedMessage(&canMessage);
    EXPECT_EQ(receiveMessageCounter, 1);
}

TEST_F(CanRawHandlerTest, ParsesReceivedShorterFrames)
{
    const CanMessage canMessage{1, {'0', '0', '0', '0', '0', '0'}};
    canRawHandler->parseReceivedMessage(&canMessage);
    EXPECT_NE(lastReceivedMessage, nullptr);
}

TEST_F(CanRawHandlerTest, ParseMultipleReceivedFrames)
{
    const CanMessage canMessage{1, {'0', '0', '0', '0', '0', '0'}};
    for (int i = 0; i < 10; i++)
    {
        canRawHandler->parseReceivedMessage(&canMessage);
    }
    EXPECT_EQ(receiveMessageCounter, 10);
}

TEST_F(CanRawHandlerTest, SendMessageSucessfuly)
{
    constexpr Core::RawCanMessage canMessage = {static_cast<std::chrono::milliseconds>(1000),
                                                {'0', '0', '0', '0', '0', '0', '0', '0'},
                                                1,
                                                8};
    eventBroker->publish(Core::SendCanMessageRawEvent(canMessage));
    EXPECT_NE(lastSentMessage, nullptr);
    EXPECT_EQ(sentMessageCounter, 1);
}

TEST_F(CanRawHandlerTest, SendMessageCorrectly)
{
    constexpr Core::RawCanMessage canMessage = {static_cast<std::chrono::milliseconds>(1000),
                                                {'1', '2', '3', '4', '5', '6', '7', '8'},
                                                1,
                                                8};
    eventBroker->publish(Core::SendCanMessageRawEvent(canMessage));
    EXPECT_EQ(lastSentMessage->getCanId(), 1);
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(lastSentMessage->getRawFrame().data[i], canMessage.data[i]);
    }
}
TEST_F(CanRawHandlerTest, SendShorterMessage)
{
    constexpr Core::RawCanMessage canMessage = {static_cast<std::chrono::milliseconds>(1000),
                                                {'1', '2', '3', '4', '5', '6', '7', '8'},
                                                1,
                                                3};
    eventBroker->publish(Core::SendCanMessageRawEvent(canMessage));
    for (int i = 0; i < canMessage.dlc; ++i)
    {
        EXPECT_EQ(lastSentMessage->getRawFrame().data[i], canMessage.data[i]);
    }
}

TEST_F(CanRawHandlerTest, HandleSendError)
{
    canRawHandler.release();
    canRawHandler =
        std::make_unique<CanRawHandler>(*eventBroker, [this](const CanMessage& canMessage) {
            lastSentMessage = std::make_unique<CanMessage>(canMessage);
            sentMessageCounter++;
            return false;
        });
    constexpr Core::RawCanMessage canMessage = {static_cast<std::chrono::milliseconds>(1000),
                                                {'0', '0', '0', '0', '0', '0', '0', '0'},
                                                1,
                                                8};
    EXPECT_NO_THROW(eventBroker->publish(Core::SendCanMessageRawEvent(canMessage)));
}
TEST_F(CanRawHandlerTest, SendMultipleMessages)
{
    constexpr Core::RawCanMessage canMessage = {static_cast<std::chrono::milliseconds>(1000),
                                                {'0', '0', '0', '0', '0', '0', '0', '0'},
                                                1,
                                                8};
    for (int i = 0; i < 10; i++)
    {
        eventBroker->publish(Core::SendCanMessageRawEvent(canMessage));
    }
    EXPECT_EQ(sentMessageCounter, 10);
}
