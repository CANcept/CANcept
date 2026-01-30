//
// Created by Adrian Rupp on 28.01.26.
//
#include "styled_frame.hpp"

#include "core/theme/theme_manager.hpp"
namespace Core {

StyledFrame::StyledFrame(QWidget* parent) : QFrame(parent)
{
    this->setObjectName("StyledFrame");

    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    setStyleSheet(QString(
                              "#StyledFrame { "
                              "background-color: %1; "
                              "border: %2px solid %3; "
                              "border-radius: %4px; "
                              "}")
                              .arg(colors.surfaceMain.name())
                              .arg(spacing.borderThin)
                              .arg(colors.borderSubtle.name(QColor::HexArgb))
                              .arg(spacing.radiusSm));
}
}  // namespace Core