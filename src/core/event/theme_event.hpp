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
#include <utility>

#include "event.hpp"
#include "settings_event.hpp"

namespace Core {

/** @brief Event to get available theme options from providers. */
struct GetAvailableThemesEvent final : public SelectProviderOptionEvent {
    explicit GetAvailableThemesEvent(std::list<SelectOption>* options)
        : SelectProviderOptionEvent(options){};
};

/** @brief Published when the user selects a different theme. */
struct ThemeChangeEvent final : public Event {
    std::string themeName;

    explicit ThemeChangeEvent(std::string themeName) : themeName(std::move(themeName)){};
};

}  // namespace Core
