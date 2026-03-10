#pragma once

#include <list>
#include <string>

#include "event.hpp"

namespace Core {

/** @brief Enumerates the available setting widget types. */
enum class SettingType { Select };

/** @brief Unique identifier for a registered setting. */
struct SettingKey {
    std::string settingId;
    std::string componentId;

    auto operator==(const SettingKey& other) const -> bool
    {
        return componentId == other.componentId && settingId == other.settingId;
    }

    SettingKey(const std::string& setting_id, const std::string& component_id)
        : settingId(setting_id), componentId(component_id)
    {
    }
};

/** @brief Represents a single option in a select-type setting. */
struct SelectOption {
    std::string value;
    std::string displayText;
};

/** @brief Published when a setting has changed. */
template <SettingType TType>
struct SettingChangedEvent final : public Event {
};

/** @brief Published when a select-type setting value has changed. */
template <>
struct SettingChangedEvent<SettingType::Select> final : public Event {
    /** @brief Key of the changed setting. */
    SettingKey key;
    /** @brief The newly selected option. */
    SelectOption value;

    SettingChangedEvent(SettingKey key, SelectOption value)
        : key(std::move(key)), value(std::move(value))
    {
    }
};

/** @brief Base event for settings that provide data. */
struct ProviderEvent : public Event {
};

/** @brief Collector event to aggregate options from various providers. */
struct SelectProviderOptionEvent : public ProviderEvent {
    /** @brief Pointer to a list populated by listeners. */
    std::list<SelectOption>* options;

    explicit SelectProviderOptionEvent(std::list<SelectOption>* options) : options(options) {};
};

}  // namespace Core
