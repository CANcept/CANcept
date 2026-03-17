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

#include "start_stop_button.hpp"

#include <QIcon>
#include <QPainter>
#include <QStyle>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "logging/constants.hpp"
namespace Logging {

StartStopButton::StartStopButton(QWidget* parent) : QPushButton(parent)
{
    setMinimumHeight(Logging::Constants::BUTTON_MIN_HEIGHT);
    setMinimumWidth(Logging::Constants::BUTTON_MIN_WIDTH);
    applyStyle();
    setRecordingState(false);
}

void StartStopButton::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString buttonStyle = QString(
                                    "QPushButton {"
                                    "   border: none;"
                                    "   border-radius: %9px;"
                                    "   font-size: %10px;"
                                    "   font-weight: %1;"
                                    "   padding: %2px %11px;"
                                    "}"
                                    "QPushButton[recording=\"false\"] {"
                                    "   background-color: %3;"
                                    "   color: %4;"
                                    "}"
                                    "QPushButton[recording=\"false\"]:hover {"
                                    "   background-color: %5;"
                                    "}"
                                    "QPushButton[recording=\"false\"]:pressed {"
                                    "   background-color: %5;"
                                    "}"
                                    "QPushButton[recording=\"true\"] {"
                                    "   background-color: %6;"
                                    "   color: %7;"
                                    "}"
                                    "QPushButton[recording=\"true\"]:hover {"
                                    "   background-color: %8;"
                                    "}"
                                    "QPushButton[recording=\"true\"]:pressed {"
                                    "   background-color: %8;"
                                    "}")
                                    .arg(spacing.fontWeightMedium)
                                    .arg(spacing.spacingLg)
                                    .arg(colors.surfacePrimary.name())
                                    .arg(colors.textPrimary.name())
                                    .arg(colors.colorPrimaryHover.name())
                                    .arg(colors.statusError.name())
                                    .arg(colors.textOnPrimary.name())
                                    .arg(colors.statusErrorHover.name())
                                    .arg(spacing.radiusMd)
                                    .arg(spacing.fontSizeMd)
                                    .arg(spacing.spacingXl * 2);
    setStyleSheet(buttonStyle);
}

void StartStopButton::setRecordingState(const bool isRecording)
{
    const auto& colors = THEME.colors();
    m_isRecording = isRecording;
    QColor color;

    if (isRecording)
    {
        setProperty("recording", true);
        color = colors.textOnPrimary;
        setText(" Stop");
    } else
    {
        setProperty("recording", false);
        color = colors.textPrimary;
        setText(" Start");
    }

    const auto& spacing = THEME.spacing();
    const int iconSize = spacing.spacingLg;
    setIconSize(QSize(iconSize, iconSize));

    const QString iconPath =
        isRecording ? Logging::Constants::STOP_ICON_PATH : Logging::Constants::START_ICON_PATH;
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull())
    {
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), color);
        painter.end();
        setIcon(QIcon(pixmap));
    }

    // Force style refresh
    style()->unpolish(this);
    style()->polish(this);
}

bool StartStopButton::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        QTimer::singleShot(0, this, [this]() { setRecordingState(m_isRecording); });
        return true;
    }
    return QPushButton::event(event);
}

}  // namespace Logging
