#include "can_raw_handler.hpp"

#include "core/macro/console_logging.hpp"
namespace CanHandler {
void CanRawHandler::parseReceivedMessage(const sockcanpp::CanMessage* canMessage)
{
    // Get message data
    Core::RawCanMessage message;
    message.messageId = static_cast<uint16_t>(canMessage->getRawFrame().can_id);
    message.receiveTime = canMessage->getTimestampOffset();
    message.dlc = static_cast<uint8_t>(canMessage->getFrameData().size());

    message.data.fill(0);
    for (int i = 0; i < 8 && i < canMessage->getFrameData().size(); i++)
    {
        message.data[i] = canMessage->getFrameData().data()[i];
    }

    // Publish message to event broker
    broker.publish(Core::ReceivedCanRawEvent(message));
}
void CanRawHandler::handleSendMessage(const Core::SendCanMessageRawEvent& event) const
{
    // Only send the actual DLC bytes
    const std::string messageData(event.canMessage.data.begin(),
                                  event.canMessage.data.begin() + event.canMessage.dlc);
    const CanMessage message{event.canMessage.messageId, messageData};

    LOG_INF("CanRawHandler", "Sending CAN message: ID=0x{:03X}, DLC={}", event.canMessage.messageId,
            event.canMessage.dlc);

    // Send message to CAN interface
    if (sendFunction(message))
    {
        LOG_INF("CanRawHandler", "CAN message sent successfully");
    } else
    {
        LOG_ERR("CanRawHandler",
                "Failed to send CAN message - device not initialized or send error");
    }
}

}  // namespace CanHandler