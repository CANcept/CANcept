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

#include "theme_manager.hpp"

#include <qlayout.h>

#include <QStyle>
#include <QWidget>

#include "color_themes.hpp"
#include "spacing_themes.hpp"
#include "style_event.hpp"

namespace Core {

auto ThemeManager::getInstance() -> ThemeManager&
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager()
{
    // Default themes
    m_spacing = std::make_unique<NormalSpacingTheme>();
    m_colors = std::make_unique<LightTheme>();
}

auto ThemeManager::spacing() const -> const SpacingTheme&
{
    return *m_spacing;
}

void ThemeManager::setSpacingTheme(std::unique_ptr<SpacingTheme> theme)
{
    m_spacing = std::move(theme);
    refreshApplication();
}

auto ThemeManager::colors() const -> const ColorTheme&
{
    return *m_colors;
}

void ThemeManager::setColorTheme(std::unique_ptr<ColorTheme> theme)
{
    m_colors = std::move(theme);
    refreshApplication();
}

void ThemeManager::refreshApplication()
{
    for (QWidget* widget : QApplication::allWidgets())
    {
        QApplication::postEvent(widget, new StyleEvent());
    }
}

}  // namespace Core
