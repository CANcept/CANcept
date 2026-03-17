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

#include "styled_checkbox.hpp"

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

StyledCheckBox::StyledCheckBox(QWidget* parent) : QCheckBox(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    applyStyle();
}

StyledCheckBox::StyledCheckBox(const QString& text, QWidget* parent) : QCheckBox(text, parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    applyStyle();
}

void StyledCheckBox::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString qss = QString(R"(
        QCheckBox {
            spacing: %1px;
            color: %2;
        }
        QCheckBox::indicator {
            width: %3px;
            height: %3px;
            border-radius: %4px;
            border: %5px solid %6;
            background-color: transparent;
        }
        QCheckBox::indicator:hover {
            border-color: %7;
        }
        QCheckBox::indicator:checked {
            background-color: %6;
            border-color: %6;
        }
        QCheckBox::indicator:indeterminate {
            background-color: %6;
            border-color: %6;
        }
        QCheckBox::indicator:focus {
            border-color: %6;
        }
    )")
                            .arg(spacing.spacingSm)
                            .arg(colors.textPrimary.name())
                            .arg(spacing.spacingSm)
                            .arg(spacing.radiusXs)
                            .arg(spacing.borderThick)
                            .arg(colors.colorPrimary.name(), colors.colorPrimaryHover.name());

    this->setStyleSheet(qss);
}

bool StyledCheckBox::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QCheckBox::event(event);
}

}  // namespace Core