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
