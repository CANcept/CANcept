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

#include "replay_control_button.hpp"

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Sending {

ReplayControlButton::ReplayControlButton(const QString& text, const Variant variant,
                                         QWidget* parent)
    : QPushButton(text, parent), m_variant(variant)
{
    const auto& spacing = THEME.spacing();
    setMinimumHeight(spacing.HeightSm);
    applyStyle();
}

void ReplayControlButton::setVariant(const Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }

    m_variant = variant;
    applyStyle();
}

void ReplayControlButton::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    QColor background = colors.colorPrimary;
    QColor hover = colors.colorPrimaryHover;
    QColor text = colors.textPrimary;
    QColor border = colors.borderSubtle;

    const QString style = QString(
                              "QPushButton {"
                              "  background-color: %1;"
                              "  color: %2;"
                              "  border: 1px solid %3;"
                              "  border-radius: %4px;"
                              "  padding: %5px %6px;"
                              "  font-size: %7px;"
                              "  font-weight: %8;"
                              "}"
                              "QPushButton:hover:!disabled {"
                              "  background-color: %9;"
                              "  border: 1px solid %3;"
                              "}"
                              "QPushButton:pressed:!disabled {"
                              "  background-color: %9;"
                              "}"
                              "QPushButton:disabled {"
                              "  background-color: %10;"
                              "  border: 1px solid %3;"
                              "  color: %11;"
                              "}")
                              .arg(background.name())
                              .arg(text.name())
                              .arg(border.name(QColor::HexArgb))
                              .arg(spacing.radiusSm)
                              .arg(spacing.spacingXs)
                              .arg(spacing.spacingXs)
                              .arg(spacing.fontSizeSm)
                              .arg(spacing.fontWeightMedium)
                              .arg(hover.name())
                              .arg(colors.surfaceSecondary.name())
                              .arg(colors.textDisabled.name());

    setStyleSheet(style);
}

auto ReplayControlButton::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QPushButton::event(event);
}

}  // namespace Sending

