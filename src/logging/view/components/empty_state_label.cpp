#include "empty_state_label.hpp"

#include "core/macro/theme.hpp"

namespace Logging {

EmptyStateLabel::EmptyStateLabel(QWidget* parent) : QLabel("No past logs", parent)
{
    setAlignment(Qt::AlignCenter);
    applyStyle();
    setVisible(false);
}

void EmptyStateLabel::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString labelStyle = QString(
                                   "QLabel {"
                                   "   border: none;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 24px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textSecondary.name());
    setStyleSheet(labelStyle);
}

}  // namespace Logging
