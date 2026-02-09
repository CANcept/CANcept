#pragma once

#include <list>
#include <string>
#include <vector>

#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_settings_registry.hpp"

namespace AppRoot {

/** @brief Provides setting data and actions to the settings view. */
class SettingsModel
{
   public:
    /** @brief Constructs the model with references to the registry and event broker. */
    SettingsModel(Core::ISettingsRegistry& registry, Core::IEventBroker& broker);
    ~SettingsModel() = default;

    SettingsModel(const SettingsModel&) = delete;
    auto operator=(const SettingsModel&) -> SettingsModel& = delete;

    /** @brief Returns all unique component IDs in registration order. */
    [[nodiscard]] auto getComponentIds() const -> std::vector<std::string>;

    /** @brief Returns settings belonging to a specific component. */
    [[nodiscard]] auto getSettingsForComponent(const std::string& componentId) const
        -> std::vector<Core::ISetting*>;

    /** @brief Returns the stored value for a setting. */
    [[nodiscard]] auto getValue(const Core::SettingKey& key) const -> std::string;

    /** @brief Updates the value of a setting and publishes a change event. */
    void setValue(const Core::SettingKey& key, const std::string& value);

    /** @brief Fetches dynamic options for a select-type setting via the event broker. */
    [[nodiscard]] auto fetchOptions(Core::ISetting* setting) -> std::list<Core::SelectOption>;

   private:
    Core::ISettingsRegistry& m_registry;
    Core::IEventBroker& m_broker;
};

}  // namespace AppRoot
