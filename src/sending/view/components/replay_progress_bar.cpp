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

#include "replay_progress_bar.hpp"

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Sending {

ReplayProgressBar::ReplayProgressBar(QWidget* parent) : QProgressBar(parent)
{
    setTextVisible(true);
    setFormat("%p%");
    setAlignment(Qt::AlignCenter);
    setMinimum(0);
    setMaximum(100);
    setValue(0);
    applyStyle();
}

void ReplayProgressBar::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const int radius = spacing.radiusSm;
    const int barHeight = spacing.spacingLg;

    const QString style = QString(
                              "QProgressBar {"
                              "  color: %1;"
                              "  background-color: %2;"
                              "  border: 1px solid %3;"
                              "  border-radius: %4px;"
                              "  min-height: %5px;"
                              "  font-weight: %7;"
                              "}"
                              "QProgressBar::chunk {"
                              "  background-color: %6;"
                              "  border-radius: %4px;"
                              "}")
                              .arg(colors.textPrimary.name())
                              .arg(colors.surfacePrimary.name())
                              .arg(colors.surfaceSecondary.name())
                              .arg(radius)
                              .arg(barHeight)
                              .arg(colors.colorPrimary.name())
                              .arg(spacing.fontWeightNormal);

    setStyleSheet(style);
}

auto ReplayProgressBar::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QProgressBar::event(event);
}

}  // namespace Sending
