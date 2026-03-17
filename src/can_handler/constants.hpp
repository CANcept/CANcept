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
#include <string>

namespace CanHandler::Constants {

/** @brief The path to the icon of the Device sleection setting */
inline const std::string DEVICE_SELECTION_ICON_PATH = ":/assets/icon/can_handler/configuration.svg";

/** @brief The Id of the module. */
inline const std::string MODULE_ID = "Can Handler";

/** @brief The Id of the setting. */
inline const std::string DEVICE_SELECTION_SETTING_ID = "Interface";

}  // namespace CanHandler::Constants