//
// Created by Florian on 14.01.2026.
//
#include "can_device_handler.hpp"
namespace CanHandler {
auto CanDeviceHandler::checkForCanMessage() const -> std::list<CanMessage>
{
    std::list<CanMessage> canMessages;
    while (true)
    {
        if (!canDriver->waitForMessages(std::chrono::milliseconds(50))) {break;}
        CanMessage readMessage = canDriver->readMessage();
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
    canDriver.reset(new CanDriver{event.deviceName, CAN_RAW});
    // canDriver->setReceiveOwnMessages(true);
}

};  // namespace CanHandler