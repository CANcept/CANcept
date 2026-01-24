//
// Created by flori on 02.01.2026.
//

#ifndef CANBUSMANAGER_CAN_DRIVER_EVENT_HPP
#define CANBUSMANAGER_CAN_DRIVER_EVENT_HPP
#include <string>

#include "event.hpp"
namespace Core {
/**
 * @brief Event, that gets published if the name of the current CAN device changes.
 */
struct CanDriverChangeEvent final : public Event {
    /**
     * @brief The new name of the CAN device.
     */
    std::string deviceName;

    explicit CanDriverChangeEvent(const std::string &deviceName) : deviceName(deviceName){};
};
}  // namespace Core
#endif  // CANBUSMANAGER_CAN_DRIVER_EVENT_HPP
