//
// Created by Florian on 14.01.2026.
//
#include "can_device_handler.hpp"
namespace CanHandler {
auto CanDeviceHandler::checkForCanMessage() const -> std::list<CanMessage>
{
    std::list<CanMessage> canMessages;
    const CanMessage emptyMessage;
    while (true)
    {
        CanMessage readMessage = canDriver->readMessage();
        if (emptyMessage == readMessage)
        {
            break;
        }
        canMessages.push_back(readMessage);
    }
    return canMessages;
}
auto CanDeviceHandler::sendCanMessage(const CanMessage& canMessage) const -> bool
{
    const ssize_t bytes_sent = canDriver->sendMessage(canMessage);
    return bytes_sent == canMessage.getFrameData().size();
}
void CanDeviceHandler::updateCanDevice(const Core::CanDriverChangeEvent& event)
{
    canDriver.reset(std::make_unique<CanDriver>(event.deviceName, CAN_RAW).get());
}

};  // namespace CanHandler