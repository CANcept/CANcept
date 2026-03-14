#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/interface/i_settings_registry.hpp"

namespace TestHelpers {

class MockSettingsRegistry final : public Core::ISettingsRegistry
{
   public:
    void registerSetting(std::unique_ptr<Core::ISetting> definition) override
    {
        settings.push_back(std::move(definition));
    }

    void setValue(const Core::SettingKey& key, const std::string& value) override
    {
        values[toStorageKey(key)] = value;
    }

    [[nodiscard]] auto getValue(const Core::SettingKey& key) const -> std::string override
    {
        const auto it = values.find(toStorageKey(key));
        return it == values.end() ? std::string{} : it->second;
    }

    [[nodiscard]] auto getSettings() const
        -> const std::vector<std::unique_ptr<Core::ISetting>>& override
    {
        return settings;
    }

    [[nodiscard]] auto getSettingsByComponent(const std::string& componentId) const
        -> std::vector<Core::ISetting*> override
    {
        std::vector<Core::ISetting*> filtered;
        for (const auto& setting : settings)
        {
            if (setting->getKey().componentId == componentId)
            {
                filtered.push_back(setting.get());
            }
        }
        return filtered;
    }

    [[nodiscard]] auto getComponentIds() const -> std::vector<std::string> override
    {
        std::vector<std::string> components;
        std::unordered_set<std::string> seen;

        for (const auto& setting : settings)
        {
            const std::string& componentId = setting->getKey().componentId;
            if (!seen.contains(componentId))
            {
                components.push_back(componentId);
                seen.insert(componentId);
            }
        }

        return components;
    }

   private:
    static auto toStorageKey(const Core::SettingKey& key) -> std::string
    {
        return key.componentId + ":" + key.settingId;
    }

    std::vector<std::unique_ptr<Core::ISetting>> settings;
    std::unordered_map<std::string, std::string> values;
};

}  // namespace TestHelpers
