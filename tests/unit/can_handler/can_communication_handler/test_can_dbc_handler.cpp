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

#include "can_handler/can_communication_handler/can_dbc_handler.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace CanHandler;

class CanDbcHandlerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canDbcHandler =
            std::make_unique<CanDbcHandler>(*eventBroker, [this](const CanMessage& canMessage) {
                sentMessageCounter++;
                return true;
            });

        receiveConnection = eventBroker->subscribe<Core::ReceivedCanDbcEvent>(
            [this](const Core::ReceivedCanDbcEvent& event) {
                lastReceivedMessage = std::make_unique<Core::DbcCanMessage>(event.canMessage);
                receiveMessageCounter++;
            });
    }

    void TearDown() override
    {
        receiveConnection.release();
        canDbcHandler.reset();
        eventBroker.reset();
    }

    // Helper function to create a simple DBC configuration
    Core::DbcConfig createSimpleDbc(uint messageId = 1, std::string messageName = "TestMessage")
    {
        return TestHelpers::DbcConfigBuilder()
            .version("1.0")
            .fileName("test.dbc")
            .node("ECU0")
            .node("ECU1")
            .node("ECU2")
            .message(TestHelpers::DbcMessageBuilder(messageId, messageName)
                         .transmitter("ECU0")
                         .signal(TestHelpers::DbcSignalBuilder("Signal1")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0.0)
                                     .range(0.0, 65535.0)
                                     .unit("unit1")
                                     .receiver("ECU1"))
                         .signal(TestHelpers::DbcSignalBuilder("Signal2")
                                     .startBit(16)
                                     .size(16)
                                     .littleEndian()
                                     .signed_()
                                     .factor(0.5)
                                     .offset(-100.0)
                                     .range(-1000.0, 1000.0)
                                     .unit("unit2")
                                     .receiver("ECU2")))
            .build();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<CanDbcHandler> canDbcHandler;
    Core::Connection receiveConnection;
    Core::Connection sendRawConnection;
    int receiveMessageCounter = 0;
    int sentMessageCounter = 0;
    std::unique_ptr<Core::SendCanMessageRawEvent> lastSentMessage;
    std::unique_ptr<Core::DbcCanMessage> lastReceivedMessage;
};

TEST_F(CanDbcHandlerTest, ConstructsSuccessfuly)
{
    EXPECT_NO_THROW(CanDbcHandler(*eventBroker, [](const CanMessage& canMessage) { return true; }));
}

TEST_F(CanDbcHandlerTest, SubscribedToSendMessageEvent)
{
    EXPECT_EQ(eventBroker.get()->subscriptionCount<Core::EncodeCanMessageDbcEvent>(), 1);
}

TEST_F(CanDbcHandlerTest, SubscribedToDbcConfigChangeEvent)
{
    EXPECT_EQ(eventBroker.get()->subscriptionCount<Core::DBCParsedEvent>(), 1);
}

TEST_F(CanDbcHandlerTest, HandlesNewDbcConfigSuccessfully)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    EXPECT_NO_THROW(eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc")));
}

TEST_F(CanDbcHandlerTest, LoadsMultipleMessagesFromDbc)
{
    TestHelpers::DbcConfigBuilder configBuilder;
    configBuilder.version("1.0").fileName("test.dbc");

    for (int i = 1; i <= 5; i++)
    {
        configBuilder.message(
            TestHelpers::DbcMessageBuilder(i, "Message" + std::to_string(i))
                .transmitter("ECU0")
                .signal(TestHelpers::DbcSignalBuilder("Signal" + std::to_string(i))
                            .startBit(0)
                            .size(8)
                            .littleEndian()
                            .unsigned_()
                            .factor(1.0)
                            .offset(0.0)
                            .range(0.0, 255.0)
                            .unit("unit")
                            .receiver("ECU")));
    }

    EXPECT_NO_THROW(eventBroker->publish(Core::DBCParsedEvent(configBuilder.build(), "test.dbc")));
}

TEST_F(CanDbcHandlerTest, SendMessageSuccessfully)
{
    // First, load a DBC config
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Create a message to send
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0},
                         Core::DbcCanSignal{.name = "Signal2", .value = 50.0}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    EXPECT_NO_THROW(eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded)));
    EXPECT_EQ(encoded.messageId, 1);
    EXPECT_EQ(encoded.dlc, 8);
}

TEST_F(CanDbcHandlerTest, SendMessageWithCorrectId)
{
    const auto dbcConfig = createSimpleDbc(5, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0}},
        .messageId = 5};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
    EXPECT_EQ(encoded.messageId, 5);
}

TEST_F(CanDbcHandlerTest, SendMultipleMessages)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    for (int i = 0; i < 10; i++)
    {
        Core::DbcCanMessage sendMessage{
            .receiveTime = std::chrono::milliseconds(1000 + i),
            .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0 + i}},
            .messageId = 1};

        Core::RawCanMessage encoded;
        eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
        EXPECT_EQ(encoded.messageId, 1);
    }
}

TEST_F(CanDbcHandlerTest, HandleSendError)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    EXPECT_NO_THROW(eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded)));
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageWithValidDbc)
{
    // First load a DBC config
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Create a raw CAN message - 8 bytes
    std::string rawData;
    rawData.push_back(0x64);  // 100 in decimal
    rawData.push_back(0x00);
    rawData.push_back(0xC8);  // 200 in decimal (for second signal)
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);

    const CanMessage message{1, rawData, std::chrono::milliseconds(0)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(1000));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_EQ(lastReceivedMessage->messageId, 1);
    EXPECT_EQ(lastReceivedMessage->receiveTime, std::chrono::nanoseconds(1000));
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageWithoutDbc)
{
    // Don't load a DBC config
    std::string rawData(8, 0x00);
    const CanMessage message{1, rawData, std::chrono::milliseconds(1000)};

    // Should not throw even without DBC config
    EXPECT_NO_THROW(canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0)));
    EXPECT_EQ(receiveMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageWithInvalidMessageId)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Try to parse a message with ID that doesn't exist in DBC
    std::string rawData(8, 0x00);
    const CanMessage message{99, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, ParseMultipleReceivedMessages)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    std::string rawData(8, 0x00);
    const CanMessage message{1, rawData, std::chrono::milliseconds(1000)};

    for (int i = 0; i < 5; i++)
    {
        canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));
    }

    EXPECT_EQ(receiveMessageCounter, 5);
}

TEST_F(CanDbcHandlerTest, DbcConfigUpdateClearsOldMessages)
{
    // Load first DBC config
    auto dbcConfig1 = createSimpleDbc(1, "Message1");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig1, "test1.dbc"));

    // Load second DBC config (should clear the first one)
    auto dbcConfig2 = createSimpleDbc(2, "Message2");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig2, "test2.dbc"));

    // Try to parse a message with ID from the first config
    std::string rawData(8, 0x00);
    const CanMessage message1{1, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message1, std::chrono::nanoseconds(0));

    // Should not receive because the config was updated
    EXPECT_EQ(receiveMessageCounter, 0);

    // Now try with the new config's message ID
    const CanMessage message2{2, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message2, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
}

TEST_F(CanDbcHandlerTest, SendMessageWithNonExistentMessageId)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Try to send a message with ID that doesn't exist
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0}},
        .messageId = 99};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
    EXPECT_EQ(sentMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, SendMessageWithPartialSignals)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Send message with only one signal
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
}

TEST_F(CanDbcHandlerTest, SendMessageWithAllSignals)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Send message with all signals
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0},
                         Core::DbcCanSignal{.name = "Signal2", .value = 50.0}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageSignalCount)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    std::string rawData(8, 0x00);
    const CanMessage message{1, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_EQ(lastReceivedMessage->signalValues.size(), 2);
}

TEST_F(CanDbcHandlerTest, DestructorCleansUpMemory)
{
    {
        auto localHandler =
            std::make_unique<CanDbcHandler>(*eventBroker, [](const CanMessage&) { return true; });

        const auto dbcConfig = createSimpleDbc(1, "TestMessage");
        eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

        // Destructor should clean up without issues
    }
    EXPECT_NO_THROW({});
}

TEST_F(CanDbcHandlerTest, HandlesEmptyDbcConfig)
{
    auto emptyConfig = TestHelpers::DbcConfigBuilder().version("1.0").fileName("empty.dbc").build();

    EXPECT_NO_THROW(eventBroker->publish(Core::DBCParsedEvent(emptyConfig, "empty.dbc")));
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageWithShortFrame)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Create a message with less than 8 bytes
    std::string shortData;
    shortData.push_back(0x64);
    shortData.push_back(0x00);
    shortData.push_back(0xC8);
    shortData.push_back(0x00);

    const CanMessage message{1, shortData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    // Should not process frames that are not 8 bytes
    EXPECT_EQ(receiveMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, ParseFrameWithTooBigCanId)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    std::string rawData(8, 0x00);

    const CanMessage message{10000, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, ParseFrameWithNegativeCanId)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    std::string rawData(8, 0x00);

    const CanMessage message{-1, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, ParseMultiplexedFrame)
{
    const auto dbcConfig = TestHelpers::DbcConfigBuilder()
                               .version("1.0")
                               .fileName("test.dbc")
                               .node("ECU0")
                               .message(TestHelpers::DbcMessageBuilder(1, "MultiplexedMessage")
                                            .transmitter("ECU0")
                                            .signal(TestHelpers::DbcSignalBuilder("MuxSignal")
                                                        .startBit(0)
                                                        .size(8)
                                                        .littleEndian()
                                                        .unsigned_()
                                                        .factor(1.0)
                                                        .offset(0.0)
                                                        .range(0.0, 255.0)
                                                        .unit("unit")
                                                        .receiver("ECU1")
                                                        .multiplexer(true))
                                            .signal(TestHelpers::DbcSignalBuilder("SignalA")
                                                        .startBit(8)
                                                        .size(16)
                                                        .littleEndian()
                                                        .unsigned_()
                                                        .factor(1.0)
                                                        .offset(0.0)
                                                        .range(0.0, 65535.0)
                                                        .unit("unit")
                                                        .receiver("ECU1")
                                                        .multiplexedBy(1))
                                            .signal(TestHelpers::DbcSignalBuilder("SignalB")
                                                        .startBit(8)
                                                        .size(16)
                                                        .littleEndian()
                                                        .unsigned_()
                                                        .factor(1.0)
                                                        .offset(0.0)
                                                        .range(0.0, 65535.0)
                                                        .unit("unit")
                                                        .receiver("ECU1")
                                                        .multiplexedBy(2)))
                               .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Create a message with less than 8 bytes
    std::string rawData;
    rawData.push_back(0x01);  // MuxSignal = 1, so SignalA should be parsed
    rawData.push_back(0x10);  // SignalA = 16
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    const CanMessage message{1, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_EQ(lastReceivedMessage->messageId, 1);
    EXPECT_EQ(lastReceivedMessage->signalValues.size(), 2);
    std::list<std::string> signalNames;
    for (const auto& signal : lastReceivedMessage->signalValues)
    {
        signalNames.push_back(signal.name);
    }
    EXPECT_EQ((std::ranges::find(signalNames, "MuxSignal") != signalNames.end()), true);
    EXPECT_EQ((std::ranges::find(signalNames, "SignalA") != signalNames.end()), true);
    EXPECT_EQ((std::ranges::find(signalNames, "SignalB") != signalNames.end()), false);
}

TEST_F(CanDbcHandlerTest, SendMessageWithTooBigId)
{
    const auto dbcConfig = createSimpleDbc(1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Send message with all signals
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0},
                         Core::DbcCanSignal{.name = "Signal2", .value = 50.0}},
        .messageId = 10000};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
    EXPECT_EQ(sentMessageCounter, 0);
}

TEST_F(CanDbcHandlerTest, SendMessageWithNegativeId)
{
    const auto dbcConfig = createSimpleDbc(-1, "TestMessage");
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Send message with all signals
    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "Signal1", .value = 100.0},
                         Core::DbcCanSignal{.name = "Signal2", .value = 50.0}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));
    EXPECT_EQ(sentMessageCounter, 0);
}

// Big Endian Signal Tests
TEST_F(CanDbcHandlerTest, SendMessageWithBigEndianSignal)
{
    const auto dbcConfig = TestHelpers::DbcConfigBuilder()
                               .version("1.0")
                               .fileName("test.dbc")
                               .node("ECU0")
                               .node("ECU1")
                               .message(TestHelpers::DbcMessageBuilder(1, "BigEndianMessage")
                                            .transmitter("ECU0")
                                            .signal(TestHelpers::DbcSignalBuilder("BigEndianSignal")
                                                        .startBit(7)
                                                        .size(16)
                                                        .bigEndian()
                                                        .unsigned_()
                                                        .factor(1.0)
                                                        .offset(0.0)
                                                        .range(0.0, 65535.0)
                                                        .unit("unit")
                                                        .receiver("ECU1")))
                               .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "BigEndianSignal", .value = 0x1234}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));

    // Big endian: MSB first, so 0x1234 should be stored as [0x12, 0x34]
    EXPECT_EQ(encoded.data[0], 0x12);
    EXPECT_EQ(encoded.data[1], 0x34);
}

TEST_F(CanDbcHandlerTest, ParseReceivedMessageWithBigEndianSignal)
{
    const auto dbcConfig = TestHelpers::DbcConfigBuilder()
                               .version("1.0")
                               .fileName("test.dbc")
                               .node("ECU0")
                               .node("ECU1")
                               .message(TestHelpers::DbcMessageBuilder(1, "BigEndianMessage")
                                            .transmitter("ECU0")
                                            .signal(TestHelpers::DbcSignalBuilder("BigEndianSignal")
                                                        .startBit(7)
                                                        .size(16)
                                                        .bigEndian()
                                                        .unsigned_()
                                                        .factor(1.0)
                                                        .offset(0.0)
                                                        .range(0.0, 65535.0)
                                                        .unit("unit")
                                                        .receiver("ECU1")))
                               .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    // Big endian: 0x1234 stored as [0x12, 0x34]
    std::string rawData;
    rawData.push_back(0x12);
    rawData.push_back(0x34);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);
    rawData.push_back(0x00);

    const CanMessage message{1, rawData, std::chrono::milliseconds(1000)};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_EQ(lastReceivedMessage->signalValues.size(), 1);
    EXPECT_EQ(lastReceivedMessage->signalValues.front().name, "BigEndianSignal");
    EXPECT_DOUBLE_EQ(lastReceivedMessage->signalValues.front().value, 0x1234);
}

TEST_F(CanDbcHandlerTest, MixedEndiannessInSameMessage)
{
    const auto dbcConfig =
        TestHelpers::DbcConfigBuilder()
            .version("1.0")
            .fileName("test.dbc")
            .node("ECU0")
            .node("ECU1")
            .message(TestHelpers::DbcMessageBuilder(1, "MixedEndianMessage")
                         .transmitter("ECU0")
                         .signal(TestHelpers::DbcSignalBuilder("LittleEndianSignal")
                                     .startBit(0)
                                     .size(16)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0.0)
                                     .range(0.0, 65535.0)
                                     .unit("unit")
                                     .receiver("ECU1"))
                         .signal(TestHelpers::DbcSignalBuilder("BigEndianSignal")
                                     .startBit(23)
                                     .size(16)
                                     .bigEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0.0)
                                     .range(0.0, 65535.0)
                                     .unit("unit")
                                     .receiver("ECU1")))
            .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "LittleEndianSignal", .value = 0x1234},
                         Core::DbcCanSignal{.name = "BigEndianSignal", .value = 0x5678}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));

    // Little endian 0x1234: [0x34, 0x12]
    EXPECT_EQ(encoded.data[0], 0x34);
    EXPECT_EQ(encoded.data[1], 0x12);

    // Big endian 0x5678: [0x56, 0x78]
    EXPECT_EQ(encoded.data[2], 0x56);
    EXPECT_EQ(encoded.data[3], 0x78);

    // Parse it back and verify
    const auto& event = encoded;
    const std::string messageData(encoded.data.begin(), encoded.data.begin() + encoded.dlc);
    const CanMessage message{event.messageId, messageData};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_EQ(lastReceivedMessage->signalValues.size(), 2);

    for (const auto& signal : lastReceivedMessage->signalValues)
    {
        if (signal.name == "LittleEndianSignal")
        {
            EXPECT_DOUBLE_EQ(signal.value, 0x1234);
        } else if (signal.name == "BigEndianSignal")
        {
            EXPECT_DOUBLE_EQ(signal.value, 0x5678);
        }
    }
}

TEST_F(CanDbcHandlerTest, BigEndian32BitSignal)
{
    const auto dbcConfig =
        TestHelpers::DbcConfigBuilder()
            .version("1.0")
            .fileName("test.dbc")
            .node("ECU0")
            .node("ECU1")
            .message(TestHelpers::DbcMessageBuilder(1, "BigEndianMessage")
                         .transmitter("ECU0")
                         .signal(TestHelpers::DbcSignalBuilder("BigEndian32BitSignal")
                                     .startBit(7)
                                     .size(32)
                                     .bigEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0.0)
                                     .range(0.0, 4294967295.0)
                                     .unit("unit")
                                     .receiver("ECU1")))
            .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    const double testValue = 0x12345678;

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "BigEndian32BitSignal", .value = testValue}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));

    // Big endian 0x12345678: [0x12, 0x34, 0x56, 0x78]
    EXPECT_EQ(encoded.data[0], 0x12);
    EXPECT_EQ(encoded.data[1], 0x34);
    EXPECT_EQ(encoded.data[2], 0x56);
    EXPECT_EQ(encoded.data[3], 0x78);

    // Parse it back
    const std::string messageData(encoded.data.begin(), encoded.data.begin() + encoded.dlc);
    const CanMessage message{encoded.messageId, messageData};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_DOUBLE_EQ(lastReceivedMessage->signalValues.front().value, testValue);
}

TEST_F(CanDbcHandlerTest, LittleEndian32BitSignal)
{
    const auto dbcConfig =
        TestHelpers::DbcConfigBuilder()
            .version("1.0")
            .fileName("test.dbc")
            .node("ECU0")
            .node("ECU1")
            .message(TestHelpers::DbcMessageBuilder(1, "LittleEndianMessage")
                         .transmitter("ECU0")
                         .signal(TestHelpers::DbcSignalBuilder("LittleEndian32BitSignal")
                                     .startBit(0)
                                     .size(32)
                                     .littleEndian()
                                     .unsigned_()
                                     .factor(1.0)
                                     .offset(0.0)
                                     .range(0.0, 4294967295.0)
                                     .unit("unit")
                                     .receiver("ECU1")))
            .build();
    eventBroker->publish(Core::DBCParsedEvent(dbcConfig, "test.dbc"));

    const double testValue = 0x12345678;

    Core::DbcCanMessage sendMessage{
        .receiveTime = std::chrono::milliseconds(1000),
        .signalValues = {Core::DbcCanSignal{.name = "LittleEndian32BitSignal", .value = testValue}},
        .messageId = 1};

    Core::RawCanMessage encoded;
    eventBroker->publish(Core::EncodeCanMessageDbcEvent(sendMessage, encoded));

    // Little endian 0x12345678: [0x78, 0x56, 0x34, 0x12]
    EXPECT_EQ(encoded.data[0], 0x78);
    EXPECT_EQ(encoded.data[1], 0x56);
    EXPECT_EQ(encoded.data[2], 0x34);
    EXPECT_EQ(encoded.data[3], 0x12);

    // Parse it back
    const auto& event = encoded;
    const std::string messageData(encoded.data.begin(), encoded.data.begin() + encoded.dlc);
    const CanMessage message{event.messageId, messageData};
    canDbcHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));

    EXPECT_EQ(receiveMessageCounter, 1);
    EXPECT_NE(lastReceivedMessage, nullptr);
    EXPECT_DOUBLE_EQ(lastReceivedMessage->signalValues.front().value, testValue);
}
