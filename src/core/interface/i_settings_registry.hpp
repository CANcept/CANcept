#pragma once

#include <memory>
#include <vector>

#include "core/dto/setting_dto.hpp"

namespace Core {

/** @brief Registry for managing application settings. */
class ISettingsRegistry
{
   public:
    virtual ~ISettingsRegistry() = default;

    /** @brief Registers a new setting by taking ownership of the definition. */
    virtual void registerSetting(std::unique_ptr<ISetting> definition) = 0;

    /** @brief Updates the value of an existing setting. */
    virtual void setValue(const SettingKey& key, const std::string& value) = 0;

    /** @brief Retrieves the current value of a setting. */
    [[nodiscard]] virtual auto getValue(const SettingKey& key) const -> std::string = 0;

    /** @brief Returns all registered settings. */
    [[nodiscard]] virtual auto getSettings() const
        -> const std::vector<std::unique_ptr<ISetting>>& = 0;

    /** @brief Filters settings by a specific component ID. */
    [[nodiscard]] virtual auto getSettingsByComponent(const std::string& componentId) const
        -> std::vector<ISetting*> = 0;

    /** @brief Returns all unique component IDs in registration order. */
    [[nodiscard]] virtual auto getComponentIds() const -> std::vector<std::string> = 0;
};

}  // namespace Core
