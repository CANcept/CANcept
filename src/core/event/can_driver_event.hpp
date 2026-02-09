#pragma once
#include <string>
#include <utility>

#include "event.hpp"
#include "settings_event.hpp"
namespace Core {
/**
 * @brief Event, that gets published if the name of the current CAN device changes.
 */
struct CanDriverChangeEvent final : public Event {
    /**
     * @brief The new name of the CAN driversNames.
     */
    std::string driverName;

    explicit CanDriverChangeEvent(std::string driverName) : driverName(std::move(driverName)){};
};
/**
 * @brief Event, that gets called to get all currently available can drivers
 */
struct GetAvailableCanDriversEvent final : public SelectProviderOptionEvent {
    explicit GetAvailableCanDriversEvent(std::list<SelectOption>* options)
        : SelectProviderOptionEvent(options){};
};
}  // namespace Core
