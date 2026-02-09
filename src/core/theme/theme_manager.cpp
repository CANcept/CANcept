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

    QApplication::processEvents();
    for (QWidget* widget : QApplication::allWidgets())
    {
        widget->style()->unpolish(widget);
        widget->setStyleSheet(widget->styleSheet());
    }

    for (QWidget* widget : QApplication::allWidgets())
    {
        if (widget->layout())
        {
            widget->layout()->invalidate();
            widget->layout()->activate();
        }
        widget->updateGeometry();
    }

    for (QWidget* widget : QApplication::allWidgets())
    {
        widget->style()->polish(widget);
        widget->update();
    }

    for (QWidget* topLevel : QApplication::topLevelWidgets())
    {
        topLevel->repaint();
    }
}

}  // namespace Core
