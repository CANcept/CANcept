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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_settings_registry.hpp"

namespace AppRoot {

/** @brief Manages the lifecycle and storage of application settings. */
class SettingsService final : public Core::ISettingsRegistry
{
   public:
    /** @brief Constructs the service with an event broker for change notifications. */
    explicit SettingsService(Core::IEventBroker& broker);
    ~SettingsService() override = default;

    void registerSetting(std::unique_ptr<Core::ISetting> definition) override;
    void setValue(const Core::SettingKey& key, const std::string& value) override;
    [[nodiscard]] auto getValue(const Core::SettingKey& key) const -> std::string override;

    [[nodiscard]] auto getSettings() const
        -> const std::vector<std::unique_ptr<Core::ISetting>>& override;

    [[nodiscard]] auto getSettingsByComponent(const std::string& componentId) const
        -> std::vector<Core::ISetting*> override;

    [[nodiscard]] auto getComponentIds() const -> std::vector<std::string> override;

   private:
    /** @brief Creates a composite map key from a SettingKey. */
    static auto makeValueKey(const Core::SettingKey& key) -> std::string;

    Core::IEventBroker& m_broker;
    std::vector<std::unique_ptr<Core::ISetting>> m_settings;
    std::vector<std::string> m_componentOrder;
    std::unordered_map<std::string, std::string> m_values;
};

}  // namespace AppRoot
