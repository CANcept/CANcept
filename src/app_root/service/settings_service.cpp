#include "settings_service.hpp"

#include <algorithm>

#include "core/event/settings_event.hpp"

namespace AppRoot {

SettingsService::SettingsService(Core::IEventBroker& broker) : m_broker(broker) {}

auto SettingsService::makeValueKey(const Core::SettingKey& key) -> std::string
{
    return key.componentId + "::" + key.settingId;
}

void SettingsService::registerSetting(std::unique_ptr<Core::ISetting> definition)
{
    if (const auto& componentId = definition->getKey().componentId;
        std::ranges::find(m_componentOrder, componentId) == m_componentOrder.end())
    {
        m_componentOrder.push_back(componentId);
    }
    m_settings.push_back(std::move(definition));
}

void SettingsService::setValue(const Core::SettingKey& key, const std::string& value)
{
    const auto valueKey = makeValueKey(key);
    if (m_values.contains(valueKey) && m_values[valueKey] == value)
    {
        return;
    }

    m_values[valueKey] = value;

    const auto it =
        std::ranges::find_if(m_settings, [&](const auto& s) { return s->getKey() == key; });

    if (it == m_settings.end())
    {
        return;
    }

    if ((*it)->getType() == Core::SettingType::Select)
    {
        m_broker.publish<Core::SettingChangedEvent<Core::SettingType::Select>>(
            Core::SettingChangedEvent<Core::SettingType::Select>{
                key, Core::SelectOption{.value = value, .displayText = ""}});
    }
}

auto SettingsService::getValue(const Core::SettingKey& key) const -> std::string
{
    const auto valueKey = makeValueKey(key);
    const auto it = m_values.find(valueKey);
    return it != m_values.end() ? it->second : std::string{};
}

auto SettingsService::getSettings() const -> const std::vector<std::unique_ptr<Core::ISetting>>&
{
    return m_settings;
}

auto SettingsService::getSettingsByComponent(const std::string& componentId) const
    -> std::vector<Core::ISetting*>
{
    std::vector<Core::ISetting*> result;
    for (const auto& s : m_settings)
    {
        if (s->getKey().componentId == componentId)
        {
            result.push_back(s.get());
        }
    }
    return result;
}

auto SettingsService::getComponentIds() const -> std::vector<std::string>
{
    return m_componentOrder;
}

}  // namespace AppRoot
