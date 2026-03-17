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

#include <QString>
#include <string>

/**
 * @namespace Constants
 * @brief Module-level constants for the application root configuration.
 */
namespace AppRoot::Constants {

/** @brief Path to the project Logo. */
inline const QString CAN_BUS_ICON_PATH = ":/assets/icon/app_root/can_bus.svg";

/** @brief Path to the settings Logo. */
inline const QString SETTINGS_ICON_PATH = ":/assets/icon/settings.svg";

/** @brief Minimum width for tab items in the tab bar */
inline constexpr int MIN_TAB_WIDTH = 120;

/** @brief Index of the settings widget in the view. */
inline constexpr int SETTINGS_VIEW_INDEX = -1;

// Theme setting
/** @brief Component ID for the appearance section in settings. */
inline const std::string THEME_COMPONENT_ID = "Appearance";

/** @brief Setting ID for the theme selector. */
inline const std::string THEME_SETTING_ID = "Theme";

/** @brief Icon path for the theme setting. */
inline const std::string THEME_ICON_PATH = ":/assets/icon/app_root/palette.svg";

/** @brief Display value for the light theme option. */
inline const std::string THEME_LIGHT = "Light";

/** @brief Display value for the dark theme option. */
inline const std::string THEME_DARK = "Dark";

/** @brief Display value for the aqua theme option. */
inline const std::string THEME_AQUA = "Aqua";

/** @brief Display value for the maroon theme option. */
inline const std::string THEME_MAROON = "Maroon";

/** @brief Display value for the dracula theme option. */
inline const std::string THEME_DRACULA = "Dracula";

/** @brief Copyright notice displayed at the bottom of the settings view. */
inline const QString COPYRIGHT_TEXT =
    "Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier";

}  // namespace AppRoot::Constants