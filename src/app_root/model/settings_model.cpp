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

#include "settings_model.hpp"

namespace AppRoot {

SettingsModel::SettingsModel(Core::ISettingsRegistry& registry, Core::IEventBroker& broker)
    : m_registry(registry), m_broker(broker)
{
}

auto SettingsModel::getComponentIds() const -> std::vector<std::string>
{
    return m_registry.getComponentIds();
}

auto SettingsModel::getSettingsForComponent(const std::string& componentId) const
    -> std::vector<Core::ISetting*>
{
    return m_registry.getSettingsByComponent(componentId);
}

auto SettingsModel::getValue(const Core::SettingKey& key) const -> std::string
{
    return m_registry.getValue(key);
}

void SettingsModel::setValue(const Core::SettingKey& key, const std::string& value)
{
    m_registry.setValue(key, value);

    for (auto* setting : m_registry.getSettingsByComponent(key.componentId))
    {
        if (setting->getKey() == key)
        {
            setting->publishChanged(m_broker, value);
            break;
        }
    }
}

auto SettingsModel::fetchOptions(Core::ISetting* setting) -> std::list<Core::SelectOption>
{
    if (!setting || setting->getType() != Core::SettingType::Select)
    {
        return {};
    }
    return setting->fetchOptions(m_broker);
}

}  // namespace AppRoot
