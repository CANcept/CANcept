#include "timer_label.hpp"

#include <QString>

#include "core/macro/theme.hpp"

namespace Logging {

TimerLabel::TimerLabel(QWidget* parent) : QLabel("00:00:00:00", parent)
{
    applyStyle();
    setVisible(false);
}

void TimerLabel::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString labelStyle = QString(
                                   "QLabel {"
                                   "   font-size: %3px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightNormal)
                                   .arg(colors.textPrimary.name())
                                   .arg(spacing.fontSizeMd);
    setStyleSheet(labelStyle);
}

void TimerLabel::updateTimer(qint64 elapsedMs)
{
    int hours = elapsedMs / 3600000;
    int minutes = (elapsedMs % 3600000) / 60000;
    int seconds = (elapsedMs % 60000) / 1000;
    int centiseconds = (elapsedMs % 1000) / 10;

    setText(QString("%1:%2:%3:%4")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'))
                .arg(centiseconds, 2, 10, QChar('0')));
}

}  // namespace Logging
