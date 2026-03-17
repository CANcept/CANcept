/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
