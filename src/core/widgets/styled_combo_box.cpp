#include "core/widgets/styled_combo_box.hpp"

#include "core/macro/theme.hpp"

namespace Core {

StyledComboBox::StyledComboBox(QWidget* parent) : QComboBox(parent)
{
    applyStyle();
}

void StyledComboBox::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString style = QString(
        "QComboBox {"
        "  background-color: %1;"
        "  color: %2;"
        "  border-radius: %3px;"
        "  padding: %4px %5px;"
        "  font-size: %6px;"
        "}"
        "QComboBox:hover {"
        "  background-color: %1;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: %7px;"
        "  padding-right: %5px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid %2;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: %1;"
        "  color: %2;"
        "  border-radius: %3px;"
        "  selection-background-color: %8;"
        "  selection-color: %2;"
        "  outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "  padding: %4px;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "  background-color: %8;"
        "}")
        .arg(colors.surfaceSecondary.name())   // %1 background
        .arg(colors.textPrimary.name())        // %2 text/arrow color
        .arg(spacing.radiusMd)                 // %3 border radius
        .arg(spacing.spacingSm)                // %4 padding vertical
        .arg(spacing.spacingMd)                // %5 padding horizontal
        .arg(spacing.fontSizeSm)               // %6 font size
        .arg(spacing.spacingXl)                // %7 dropdown width
        .arg(colors.surfacePrimary.name());    // %8 hover/selection bg

    setStyleSheet(style);
}

}  // namespace Core
