//
// Created by Florian on 14.01.2026.
//
#include "can_raw_handler.hpp"
namespace CanHandler {
void CanRawHandler::parseReceivedMessage(const sockcanpp::CanMessage* canMessage)
{
    // Get message data
    Core::RawCanMessage message;
    message.messageId = (char)canMessage->getRawFrame().can_id;
    message.receiveTime = canMessage->getTimestampOffset();
    std::array<char, 8> data;
    for (int i = 0; i < 8 && i < canMessage->getFrameData().size(); i++)
    {
        data[i] = canMessage->getFrameData().data()[i];
    }
    message.data = data;

    // Publish message to event broker
    broker.publish(Core::ReceivedCanRawEvent(message));
}
void CanRawHandler::handleSendMessage(const Core::SendCanMessageRawEvent& event) const
{
    // Convert message data to string
    std::string messageData = "";
    for (const char dataChar : event.canMessage.data)
    {
        messageData += dataChar;
    }
    const CanMessage message{event.canMessage.messageId, messageData};

    // Send message to CAN interface
    sendFunction(message);
}

}  // namespace CanHandler