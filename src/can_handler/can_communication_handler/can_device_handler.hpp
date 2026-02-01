//
// Created by flori on 03.01.2026.
//

#ifndef CANBUSMANAGER_CAN_DEVICE_HANDLER_H
#define CANBUSMANAGER_CAN_DEVICE_HANDLER_H
#include <ifaddrs.h>

#include <CanDriver.hpp>
#include <list>
using sockcanpp::CanDriver;
using sockcanpp::CanMessage;
#include "core/event/can_driver_event.hpp"
#include "core/interface/i_event_broker.hpp"
namespace CanHandler {
class CanDeviceHandler
{
   public:
    explicit CanDeviceHandler(Core::IEventBroker& event_broker)
    {
        canDriverChangeEventConnection = event_broker.subscribe<Core::CanDriverChangeEvent>(
            [this](const Core::CanDriverChangeEvent& event) -> void { updateCanDevice(event); });
        getAvailableCanDevicesEventConnection =
            event_broker.subscribe<Core::GetAvailableDriversEvent>(
                [this](const Core::GetAvailableDriversEvent& event) -> void {
                    getAvailableCanDevices(event);
                });
    };

    /**
     * @brief Checks if new messages were received over the CAN bus.
     * @return A list of received messages
     */
    auto checkForCanMessage() const -> std::list<CanMessage>;

    /**
     * @brief Sends a message to the current can driver
     * @param canMessage The message to be sent
     * @return A bool indicating if the sending was successful
     */
    auto sendCanMessage(const CanMessage& canMessage) const -> bool;

   private:
    /**
     * @brief Called, when a @code Core::CanDriverChangeEvent@endcode is registered. Updates the can
     * driver accordingly
     * @param event The event, that contains the new device name
     *
     */
    void updateCanDevice(const Core::CanDriverChangeEvent& event);

    /**
     * @brief Called, when a Core::GetAvailableDriversEven is registered. Adds all can drivers to
     * the list in the event
     * @param event The event, that will contain the driver names
     */
    void getAvailableCanDevices(const Core::GetAvailableDriversEvent& event);

    /**
     * @brief The current configuration of the can driver, containing the device info for libsockcan
     */
    std::unique_ptr<CanDriver> canDriver;
    /**
     * @brief A connection containing the subscription to the can driver change event
     */
    Core::Connection canDriverChangeEventConnection;
    /**
     * @brief A connection conatining the subscription to the get available van devices event
     */
    Core::Connection getAvailableCanDevicesEventConnection;
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_CAN_DEVICE_HANDLER_H
