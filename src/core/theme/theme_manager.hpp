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
#include <QApplication>
#include <QObject>
#include <memory>

#include "color_themes.hpp"
#include "spacing_themes.hpp"

namespace Core {

/**
 * @class ThemeManager
 * @brief Central manager for application-wide themes.
 *
 * ThemeManager holds the current color and spacing themes.
 *
 * This class follows the singleton pattern: there is only one instance in the app.
 */
class ThemeManager final : public QObject
{
   public:
    /**
     * @brief Retrieves the singleton instance of ThemeManager.
     * @return Reference to the ThemeManager instance.
     */
    static auto getInstance() -> ThemeManager&;

    [[nodiscard]] auto spacing() const -> const SpacingTheme&;

    /**
     * @brief Sets a new spacing theme and triggers a full app refresh.
     * @param theme Unique pointer to a SpacingTheme.
     */
    void setSpacingTheme(std::unique_ptr<SpacingTheme> theme);

    [[nodiscard]] auto colors() const -> const ColorTheme&;

    /**
     * @brief Sets a new color theme and triggers a full app refresh.
     * @param theme Unique pointer to a ColorTheme.
     */
    void setColorTheme(std::unique_ptr<ColorTheme> theme);

    // Non-copyable
    ThemeManager(const ThemeManager&) = delete;
    auto operator=(const ThemeManager&) -> ThemeManager& = delete;

   private:
    ThemeManager();
    ~ThemeManager() override = default;

    /**
     * @brief Forces a repaint of all widgets in the application.
     */
    void refreshApplication();

    std::unique_ptr<SpacingTheme> m_spacing;
    std::unique_ptr<ColorTheme> m_colors;
};

}  // namespace Core
