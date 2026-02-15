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
