#include "theme_manager.hpp"

#include <QWidget>

#include "color_themes.hpp"
#include "spacing_themes.hpp"

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
    // Reapply global stylesheet if any
    qApp->setStyleSheet(qApp->styleSheet());

    // Repaint all widgets
    for (QWidget* w : QApplication::allWidgets())
    {
        w->update();  // schedules repaint
    }
}

}  // namespace Core
