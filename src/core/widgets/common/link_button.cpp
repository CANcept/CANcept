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

#include "link_button.hpp"

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

LinkButton::LinkButton(const QString& text, QWidget* parent) : QPushButton(text, parent)
{
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
    applyStyle();
}

void LinkButton::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const QString buttonStyle = QString(
                                    "QPushButton {"
                                    "  background: transparent;"
                                    "  border: none;"
                                    "  color: %1;"
                                    "  text-decoration: underline;"
                                    "  font-size: %2px;"
                                    "  padding: %3px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "  color: %4;"
                                    "}"
                                    "QPushButton:disabled {"
                                    "  color: %5;"
                                    "}")
                                    .arg(colors.textSecondary.name())
                                    .arg(spacing.fontSizeMd)
                                    .arg(spacing.spacingSm)
                                    .arg(colors.textPrimary.name())
                                    .arg(colors.textDisabled.name());
    setStyleSheet(buttonStyle);
}

bool LinkButton::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QPushButton::event(event);
}

}  // namespace Core
