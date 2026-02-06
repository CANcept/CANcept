#include "can_device_handler.hpp"

#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "can_handler/constants.hpp"
#include "core/macro/console_logging.hpp"

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
    LOG_INF("CanDeviceHandler", "Initializing CAN driver with device: {}", event.driverName);
    try
    {
        canDriver.reset(new CanDriver{event.driverName, CAN_RAW});
        LOG_INF("CanDeviceHandler", "CAN driver initialized successfully on {}", event.driverName);
        // canDriver->setReceiveOwnMessages(true);
    } catch (const std::exception& e)
    {
        LOG_ERR("CanDeviceHandler", "Failed to initialize CAN driver: {}", e.what());
        canDriver.reset();
    }
}

void CanDeviceHandler::getAvailableCanDevices(const Core::GetAvailableCanDriversEvent& event)
{
    ifaddrs* firstInterface;
    if (getifaddrs(&firstInterface) == -1)
    {
        LOG_ERR("CanHandler",
                "Could not access network devices for filtering available CAN devices")
        return;
    }
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0)
    {
        freeifaddrs(firstInterface);
        LOG_ERR("CanHandler", "Failed to create socket for CAN device detection");
        return;
    }

    for (const ifaddrs* interface = firstInterface; interface != nullptr;
         interface = interface->ifa_next)
    {
        if (!interface->ifa_name) continue;

        ifreq requestedInterface{};
        std::strncpy(requestedInterface.ifr_name, interface->ifa_name, IFNAMSIZ - 1);

        if (ioctl(sock, SIOCGIFHWADDR, &requestedInterface) == 0)
        {
            if (requestedInterface.ifr_ifru.ifru_hwaddr.sa_family == ARPHRD_CAN)
            {
                event.options->push_back(Core::SelectOption{std::string(interface->ifa_name),
                                                            std::string(interface->ifa_name)});
            }
        }
    }
    freeifaddrs(firstInterface);
    close(sock);
}

void CanDeviceHandler::registerSettings(Core::ISettingsRegistry& registry)
{
    registry.registerSetting(
        std::make_unique<
            Core::SettingDefinition<Core::SettingType::Select, Core::GetAvailableCanDriversEvent,
                                    Core::CanDriverChangeEvent>>(
            Core::SettingKey{Constants::DEVICE_SELECTION_SETTING_ID, Constants::MODULE_ID},
            Constants::DEVICE_SELECTION_ICON_PATH,
            Core::TypeTraits<Core::SettingType::Select, Core::GetAvailableCanDriversEvent>{
                "Select Interface"}));
}

};  // namespace CanHandler