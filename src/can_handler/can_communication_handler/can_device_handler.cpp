#include "can_device_handler.hpp"

#include <net/if.h>

#include "core/macro/console_logging.hpp"
#include "spdlog/fmt/bundled/base.h"
namespace CanHandler {
auto CanDeviceHandler::checkForCanMessage() const -> std::list<CanMessage>
{
    std::list<CanMessage> canMessages;
    while (true)
    {
        if (canDriver.get() == nullptr)
        {
            return {};
        }
        if (!canDriver->waitForMessages(std::chrono::milliseconds(50)))
        {
            break;
        }
        CanMessage readMessage = canDriver->readMessage();
        canMessages.push_back(readMessage);
    }
    return canMessages;
}
auto CanDeviceHandler::sendCanMessage(const CanMessage& canMessage) const -> bool
{
    if (canDriver.get() == nullptr)
    {
        LOG_ERR("CanDeviceHandler", "Cannot send message: CAN driver not initialized");
        return false;
    }
    const ssize_t bytes_sent = canDriver->sendMessage(canMessage);

    // can id + dlc + data
    const bool success = bytes_sent == 16;

    if (!success)
    {
        LOG_ERR("CanDeviceHandler", "Send failed: expected {} bytes (CAN_MTU), sent {} bytes", 16,
                bytes_sent);
    }

    return success;
}
void CanDeviceHandler::updateCanDevice(const Core::CanDriverChangeEvent& event)
{
    canDriver.reset(new CanDriver{event.driverName, CAN_RAW});
    // canDriver->setReceiveOwnMessages(true);
    LOG_INF("CanDeviceHandler", "Initializing CAN driver with device: {}", event.deviceName);
    try
    {
        canDriver.reset(new CanDriver{event.deviceName, CAN_RAW});
        LOG_INF("CanDeviceHandler", "CAN driver initialized successfully on {}", event.deviceName);
        // canDriver->setReceiveOwnMessages(true);
    } catch (const std::exception& e)
    {
        LOG_ERR("CanDeviceHandler", "Failed to initialize CAN driver: {}", e.what());
        canDriver.reset();
    }
}

void CanDeviceHandler::getAvailableCanDevices(const Core::GetAvailableDriversEvent& event)
{
    ifaddrs* firstInterface;
    if (getifaddrs(&firstInterface) == -1) {
        LOG_ERR("CanHandler", "Could find can drivers")
        return;
    }
    for (const ifaddrs* interface = firstInterface; interface != nullptr; interface = interface->ifa_next)
    {
        if (!interface->ifa_addr)
        {
            continue;
        }
        if (interface->ifa_addr->sa_family == AF_CAN)
        {
            event.driversNames->push_back(std::string(interface->ifa_name));
        }
    }
    freeifaddrs(firstInterface);
}
};  // namespace CanHandler