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

#include "app_root/constants.hpp"
#include "core/dto/setting_dto.hpp"
#include "core/event/settings_event.hpp"
#include "core/event/theme_event.hpp"

namespace TestHelpers {

/**
 * @brief Creates the standard app-root theme setting.
 */
inline auto makeThemeSetting()
{
    return std::make_unique<Core::SettingDefinition<
        Core::SettingType::Select, Core::GetAvailableThemesEvent, Core::ThemeChangeEvent>>(
        Core::SettingKey{AppRoot::Constants::THEME_SETTING_ID,
                         AppRoot::Constants::THEME_COMPONENT_ID},
        AppRoot::Constants::THEME_ICON_PATH,
        Core::TypeTraits<Core::SettingType::Select, Core::GetAvailableThemesEvent>{"Select Theme"});
}

/**
 * @brief Creates a generic Select setting for a given key.
 */
inline auto makeSelectSetting(const std::string& settingId, const std::string& componentId)
{
    return std::make_unique<
        Core::SettingDefinition<Core::SettingType::Select, Core::SelectProviderOptionEvent>>(
        Core::SettingKey{settingId, componentId}, "icon",
        Core::TypeTraits<Core::SettingType::Select, Core::SelectProviderOptionEvent>{
            "Placeholder"});
}

}  // namespace TestHelpers
