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
    const auto& spacing = THEME.spacing();
    setIconSize(QSize(20, 20));
    // setFixedSize(75, 35);
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
                                    "   padding: %2px %2px;"
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
                                    .arg(spacing.spacingMd)
                                    .arg(colors.surfacePrimary.name())
                                    .arg(colors.textPrimary.name())
                                    .arg(colors.colorPrimaryHover.name())
                                    .arg(colors.statusError.name())
                                    .arg(colors.textOnPrimary.name())
                                    .arg(colors.statusErrorHover.name())
                                    .arg(spacing.radiusMd)
                                    .arg(spacing.fontSizeMd);
    setStyleSheet(buttonStyle);
    updateButtonIcon();
}

void StartStopButton::setRecordingState(bool isRecording)
{
    m_isRecording = isRecording;
    updateButtonIcon();

    // Force style refresh
    style()->unpolish(this);
    style()->polish(this);
}

bool StartStopButton::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QPushButton::event(event);
}

void StartStopButton::updateButtonIcon()
{
    const auto& colors = THEME.colors();
    if (m_isRecording)
    {
        QPixmap pixmap(Constants::STOP_ICON_PATH);
        if (!pixmap.isNull())
        {
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), colors.textSecondary);
            painter.end();
            setIcon(QIcon(pixmap));
        }
    } else
    {
        QPixmap pixmap(Constants::START_ICON_PATH);
        if (!pixmap.isNull())
        {
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), colors.textSecondary);
            painter.end();
            setIcon(QIcon(pixmap));
        }
    }
}
}  // namespace Logging
