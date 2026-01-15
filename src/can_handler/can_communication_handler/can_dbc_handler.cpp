//
// Created by Florian on 14.01.2026.
//
#include "can_dbc_handler.hpp"
namespace CanHandler {
void CanDbcHandler::parseReceivedMessage(const sockcanpp::CanMessage* canMessage)
{
    // Get right message description
    Core::DbcMessageDescription currentMessageDescription;
    for (const Core::DbcMessageDescription messageDescription : currentConfig.messageDefinitions)
    {
        if (messageDescription.messageId == canMessage->getRawFrame().can_id)
        {
            currentMessageDescription = messageDescription;
            break;
        }
    }

    // Check if message length = 8
    if (canMessage->getRawFrame().len != 8) return;

    // Read raw data frames in little and big endian order
    u_int64_t dataLittleEndian = 0, dataBigEndian = 0;
    for (int i = 0; i < 8; i++)
    {
        dataLittleEndian += canMessage->getRawFrame().data[i] << (i * 8);
        dataBigEndian = (dataBigEndian << 8) + canMessage->getRawFrame().data[i];
    }
    // Multiplex signal initialization
    std::list<Core::DbcSignalDescription> multiplexedSignals;
    int multiplexorValue = -1;

    // Parsed can message initialization
    Core::DbcCanMessage receivedMessage{
        .receiveTime = canMessage->getTimestampOffset(),
        .messageId = static_cast<char>(canMessage->getRawFrame().can_id)};

    // Parse signals
    for (const Core::DbcSignalDescription signalDescription :
         currentMessageDescription.signalDescriptions)
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
        rawValue =
            dataLittleEndian << (sizeof(int64_t) - signal.startBit -
                                 signal.signalSize)  // remove all data in front of data
            >>
            (sizeof(int64_t) - signal.signalSize);  // shift back (sizeof(int64_t) - signal.startBit
                                                    // - signal.signalSize), remove all data behind
                                                    // data ( +signal.startBit)
    } else                                          // big endian
    {
        rawValue =
            dataBigEndian << (sizeof(int64_t) -
                              signal.startBit)  // remove all data in front of data
            >> (sizeof(int64_t) -
                signal.signalSize);  // shift back (sizeof(int64_t) - signal.startBit), remove all
                                     // data behind data ( +signal.startBit - signal.signalSize)
    }
    if (signal.valueType)  // signed
    {
        rawValue = rawValue << (sizeof(int64_t) - signal.signalSize) >>
                   (sizeof(int64_t) - signal.signalSize);  // let the shift operator sign the value
    }
    return rawValue * signal.factor + signal.offset;
}

void CanDbcHandler::handleSendMessage(const Core::SendCanMessageDbcEvent& event)
{
    // Get right message description
    Core::DbcMessageDescription currentMessageDescription;
    for (const Core::DbcMessageDescription messageDescription : currentConfig.messageDefinitions)
    {
        if (messageDescription.messageId == event.canMessage.messageId)
        {
            currentMessageDescription = messageDescription;
            break;
        }
    }
    // Data frames for data to be parsed
    u_int64_t dataLittleEndian = 0, dataBigEndian = 0;

    // Signals to raw data
    for (Core::DbcCanSignal signal : event.canMessage.signalValues)
    {
        for (Core::DbcSignalDescription signalDescription :
             currentMessageDescription.signalDescriptions)
        {
            if (signalDescription.signalName == signal.name)
            {
                parseSendSignal(signalDescription, dataLittleEndian, dataBigEndian, signal.value);
                break;
            }
        }
    }

    // Add raw data values
    std::string data;
    for (int i = 0; i < 8; i++)
    {
        data += static_cast<char>(static_cast<char>(dataLittleEndian >> (8 * i)) +
                                  static_cast<char>(dataBigEndian >> (8 * (8 - i))));
    }

    // Transform to CAN message
    const CanMessage message{event.canMessage.messageId, data};

    // Send message to CAN interface
    sendFunction(message);
}

void CanDbcHandler::parseSendSignal(const Core::DbcSignalDescription& signal,
                                    u_int64_t& dataLittleEndian, u_int64_t& dataBigEndian,
                                    const double& value)
{
    u_int64_t rawValue = static_cast<int64_t>((value - signal.offset) / signal.factor);
    if (!signal.byteOrder)  // little endian
    {
        dataLittleEndian =
            dataLittleEndian +
            (rawValue << (sizeof(int64_t) - signal.signalSize)  // cut of eventual 1 in sign
             >> (sizeof(int64_t) - signal.signalSize -
                 signal.startBit)  // shift back (sizeof(int64_t) - signal.signalSize) and in the
                                   // other direction for the starting bit (- signal.startBit)
            );
    } else  // big endian
    {
        dataBigEndian =
            dataBigEndian +
            (rawValue << (sizeof(int64_t) - signal.signalSize)  // cut of eventual 1 in sign
             >> (sizeof(int64_t) -
                 signal.startBit)  // shift back (sizeof(int64_t) - signal.signalSize) and in the
                                   // other direction for the starting bit (- (signal.startBit -
                                   // signal.signalSize))
            );
    }
}

void CanDbcHandler::handleNewDbc(const Core::DBCParsedEvent& event)
{
    currentConfig = event.config;
}
}  // namespace CanHandler