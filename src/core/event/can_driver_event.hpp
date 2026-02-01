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
     * @brief The new name of the CAN driversNames.
     */
    std::string driverName;

    explicit CanDriverChangeEvent(const std::string &driverName) : driverName(driverName){};
};
/**
 * @brief Event, that gets called to get all currently available can drivers
 */
struct GetAvailableDriversEvent final : public Event
{
    /**
     * @brief After the event returns this list contains all available can drivers
     */
    std::unique_ptr<std::list<std::string>> driversNames;

    explicit GetAvailableDriversEvent() : driversNames(std::make_unique<std::list<std::string>>()) {}
};
}  // namespace Core
#endif  // CANBUSMANAGER_CAN_DRIVER_EVENT_HPP
