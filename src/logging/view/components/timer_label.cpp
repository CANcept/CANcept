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

#include "timer_label.hpp"

#include <QString>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

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

auto TimerLabel::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QLabel::event(event);
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
