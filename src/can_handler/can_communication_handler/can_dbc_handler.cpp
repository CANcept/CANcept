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

#include "can_dbc_handler.hpp"

#include "core/macro/console_logging.hpp"

namespace CanHandler {
void CanDbcHandler::parseReceivedMessage(const sockcanpp::CanMessage* canMessage)
{
    // Lock the mutex for data safety
    std::shared_lock guard(dbcMutex);
    // Get the right message description
    if (canMessage->getRawFrame().can_id >= dbcMessages.size())
    {
        return;
    }
    const Core::DbcMessageDescription* currentMessageDescription =
        dbcMessages.at(canMessage->getRawFrame().can_id);
    if (currentMessageDescription == nullptr)  // no fitting message description
    {
        return;
    }

    // Check if message length = 8
    if (canMessage->getRawFrame().len != 8) return;

    // Read raw data frames in little and big endian order
    u_int64_t dataLittleEndian = 0, dataBigEndian = 0;
    for (int i = 0; i < 8; i++)
    {
        dataLittleEndian += static_cast<u_int64_t>(canMessage->getRawFrame().data[i]) << (i * 8);
        dataBigEndian = (dataBigEndian << 8) + canMessage->getRawFrame().data[i];
    }
    // Multiplex signal initialization
    std::list<Core::DbcSignalDescription> multiplexedSignals;
    int multiplexorValue = -1;

    // Parsed can message initialization
    Core::DbcCanMessage receivedMessage{
        .receiveTime = canMessage->getTimestampOffset(),
        .messageId = static_cast<uint16_t>(canMessage->getRawFrame().can_id)};

    // Parse signals
    for (const Core::DbcSignalDescription signalDescription :
         currentMessageDescription->signalDescriptions)
    {
        if (signalDescription.multiplexedBy != -1)
        {
            multiplexedSignals.push_back(signalDescription);
            continue;
        }
        if (signalDescription.multiplexer)
        {
            multiplexorValue = static_cast<int>(
                parseReceivedSignal(signalDescription, dataLittleEndian, dataBigEndian));
            receivedMessage.signalValues.push_back(
                Core::DbcCanSignal{.name = signalDescription.signalName,
                                   .value = static_cast<double>(multiplexorValue)});
            continue;
        }
        receivedMessage.signalValues.push_back(Core::DbcCanSignal{
            .name = signalDescription.signalName,
            .value = parseReceivedSignal(signalDescription, dataLittleEndian, dataBigEndian)});
    }

    // Parse multiplexed signals
    for (const Core::DbcSignalDescription signalDescription : multiplexedSignals)
    {
        if (signalDescription.multiplexedBy == multiplexorValue)
        {
            receivedMessage.signalValues.push_back(Core::DbcCanSignal{
                .name = signalDescription.signalName,
                .value = parseReceivedSignal(signalDescription, dataLittleEndian, dataBigEndian)});
        }
    }

    // Publish to the event broker
    broker.publish(Core::ReceivedCanDbcEvent(receivedMessage));
}
auto CanDbcHandler::parseReceivedSignal(const Core::DbcSignalDescription& signal,
                                        const u_int64_t& dataLittleEndian,
                                        const u_int64_t& dataBigEndian) -> double
{
    int64_t rawValue = 0;
    if (!signal.byteOrder)  // little endian
    {
        rawValue = dataLittleEndian << (64 - signal.startBit -
                                        signal.signalSize)  // remove all data in front of data
                   >> (64 - signal.signalSize);  // shift back (sizeof(int64_t) - signal.startBit
                                                 // - signal.signalSize), remove all data behind
                                                 // data ( +signal.startBit)
    } else                                       // big endian
    {
        // DBC bit N sits at linear position (56 - 8*(N/8) + N%8) in a MSB-first 64-bit word.
        // Shifting by 63 - pos(startBit) aligns the signal MSB to bit 63;
        // >> (64 - signalSize) then isolates the signal at bits [signalSize-1 : 0]
        const uint8_t linearStart = 63 - (56 - 8 * (signal.startBit / 8) + signal.startBit % 8);
        rawValue = static_cast<int64_t>((dataBigEndian << linearStart) >> (64 - signal.signalSize));
    }
    if (signal.valueType)  // signed
    {
        rawValue = rawValue << (64 - signal.signalSize) >>
                   (64 - signal.signalSize);  // let the shift operator sign the value
    }
    return rawValue * signal.factor + signal.offset;
}

void CanDbcHandler::handleEncodeMessage(const Core::EncodeCanMessageDbcEvent& event)
{
    LOG_INF("CanDbcHandler", "handleSendMessage called for message ID 0x{:X}",
            event.canMessage.messageId);

    std::shared_lock lock(dbcMutex);

    // Get right message description
    if (event.canMessage.messageId >= dbcMessages.size())
    {
        LOG_ERR("CanDbcHandler", "Message ID 0x{:X} >= array size {}", event.canMessage.messageId,
                dbcMessages.size());
        return;
    }
    const Core::DbcMessageDescription* currentMessageDescription =
        dbcMessages[event.canMessage.messageId];
    if (currentMessageDescription == nullptr)
    {
        LOG_ERR("CanDbcHandler", "No message description found for ID 0x{:X}",
                event.canMessage.messageId);
        return;
    }

    Core::DbcCanMessage messageToEncode = event.canMessage;

    // Data frames for little-endian and big-endian signals
    u_int64_t dataLittleEndian = 0;
    u_int64_t dataBigEndian = 0;

    // Encode all signals into the appropriate data frame
    for (const auto& [name, value] : messageToEncode.signalValues)
    {
        for (const Core::DbcSignalDescription& signalDescription :
             currentMessageDescription->signalDescriptions)
        {
            if (signalDescription.signalName == name)
            {
                parseSendSignal(signalDescription, dataLittleEndian, dataBigEndian, value);
                break;
            }
        }
    }

    LOG_INF("CanDbcHandler", "Encoded data LE=0x{:016X} BE=0x{:016X}", dataLittleEndian,
            dataBigEndian);

    // Build raw byte array from encoded signals
    uint16_t id = messageToEncode.messageId;
    uint8_t dlc = 8;
    std::array<char, 8> data{};
    for (int i = 0; i < 8; i++)
    {
        // Extract byte i from little-endian representation
        const auto byteLittleEndian = static_cast<uint8_t>((dataLittleEndian >> (i * 8)) & 0xFF);
        // Extract byte i from big-endian representation
        const auto byteBigEndian = static_cast<uint8_t>((dataBigEndian >> ((7 - i) * 8)) & 0xFF);
        // Combine with OR (signals should not overlap in well-formed DBC)
        data[i] = static_cast<uint8_t>(byteLittleEndian | byteBigEndian);
    }

    // Populate the output reference with the encoded raw frame
    event.encodedMessage.messageId = id;
    event.encodedMessage.dlc = dlc;
    event.encodedMessage.data = data;
}

void CanDbcHandler::parseSendSignal(const Core::DbcSignalDescription& signal,
                                    u_int64_t& dataLittleEndian, u_int64_t& dataBigEndian,
                                    const double& value)
{
    // Clamp value to signal's physical range
    double clampedValue = value;
    if (clampedValue < signal.minimum)
    {
        clampedValue = signal.minimum;
    } else if (clampedValue > signal.maximum)
    {
        clampedValue = signal.maximum;
    }

    // Convert physical value to raw value
    const int64_t rawValue = static_cast<int64_t>((clampedValue - signal.offset) / signal.factor);

    // Create mask for signal size
    const uint64_t mask = (signal.signalSize >= 64) ? ~0ULL : ((1ULL << signal.signalSize) - 1);
    const uint64_t maskedRawValue = static_cast<uint64_t>(rawValue) & mask;

    if (!signal.byteOrder)  // Little endian
    {
        // Decode: rawValue = dataLE << (64 - startBit - signalSize) >> (64 - signalSize)
        // This extracts signal from bits [startBit, startBit + signalSize - 1]
        // Encode (inverse): place value at those bits
        dataLittleEndian |= (maskedRawValue << signal.startBit);
    } else  // Big endian (Motorola)
    {
        // DBC bit N maps to linear position 56 - 8*(N/8) + N%8 in a MSB-first 64-bit word.
        const uint8_t linearStart = 63 - (56 - 8 * (signal.startBit / 8) + signal.startBit % 8);
        dataBigEndian |= static_cast<uint64_t>(maskedRawValue)
                         << (64 - linearStart - signal.signalSize);
    }
}

void CanDbcHandler::handleNewDbc(const Core::DBCParsedEvent& event)
{
    LOG_INF("CanDbcHandler", "Received new DBC config with {} messages",
            event.config.messageDefinitions.size());

    std::unique_lock guard(dbcMutex);

    // Clear array and free memory
    for (auto& dbcMessage : dbcMessages)
    {
        if (dbcMessage != nullptr)
        {
            delete dbcMessage;
            dbcMessage = nullptr;
        }
    }

    // Input message definitions into array
    int loaded = 0;
    for (const auto& [messageId, messageName, messageSize, transmitterName, signalDescriptions] :
         event.config.messageDefinitions)
    {
        if (messageId < dbcMessages.size())
        {
            dbcMessages[messageId] =
                new Core::DbcMessageDescription{.messageId = messageId,
                                                .messageName = messageName,
                                                .messageSize = messageSize,
                                                .transmitterName = transmitterName,
                                                .signalDescriptions = signalDescriptions};
            loaded++;
        }
    }

    LOG_INF("CanDbcHandler", "DBC config loaded: {} messages stored in array", loaded);
}
CanDbcHandler::~CanDbcHandler()
{
    for (const auto& dbcMessage : dbcMessages)
    {
        if (dbcMessage != nullptr)
        {
            delete dbcMessage;
        }
    }
    dbcConfigChangeConnection.release();
    dbcSendEventConnection.release();
}

}  // namespace CanHandler