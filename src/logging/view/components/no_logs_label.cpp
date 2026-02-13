#include "no_logs_label.hpp"

#include "core/macro/theme.hpp"

namespace Logging {

NoLogsLabel::NoLogsLabel(QWidget* parent) : QLabel("No past logs", parent)
{
    setAlignment(Qt::AlignCenter);
    applyStyle();
    setVisible(false);
}

void NoLogsLabel::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString labelStyle = QString(
                                   "QLabel {"
                                   "   border: none;"
                                   "   font-size: %3px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textSecondary.name())
                                   .arg(spacing.fontSizeLg);
    setStyleSheet(labelStyle);
}

}  // namespace Logging
