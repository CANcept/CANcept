#pragma once

#include <memory>
#include <string>

#include "app_root/constants.hpp"
#include "core/dto/setting_dto.hpp"
#include "core/event/settings_event.hpp"
#include "core/event/theme_event.hpp"

namespace TestHelpers {

/**
 * @brief Creates the standard app-root theme setting (Select, driven by GetAvailableThemesEvent).
 * Matches what AppRoot::bootstrap() registers.
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
 * @brief Creates a generic Select setting for a given key. Use when you need arbitrary
 * settings in tests without coupling to app-root constants.
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
