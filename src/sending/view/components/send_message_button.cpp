#include "send_message_button.hpp"

#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "sending/constants.hpp"

namespace Sending {

SendMessageButton::SendMessageButton(QWidget* parent) : QPushButton(parent)
{
    setText(Constants::SEND_BUTTON_TEXT);
    setMinimumHeight(Constants::SEND_BUTTON_MIN_HEIGHT);
    setMinimumWidth(Constants::SEND_BUTTON_MIN_WIDTH);

    applyStyle();
}

void SendMessageButton::applyStyle()
{
    // Tint icon with current theme color
    QPixmap pixmap(Constants::SEND_BUTTON_ICON_PATH);
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), THEME.colors().textPrimary);
    painter.end();
    setIcon(QIcon(pixmap));

    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Set overall styles to align with the project
    const int iconSize = spacing.spacingLg;
    setIconSize(QSize(iconSize, iconSize));

    const QString buttonStyle = QString(
                                    "QPushButton {"
                                    "  background-color: %1;"
                                    "  color: %2;"
                                    "  border: none;"
                                    "  border-radius: %3px;"
                                    "  padding: %4px %5px;"
                                    "  font-weight: %6;"
                                    "  font-size: %7px;"
                                    "  text-align: center;"
                                    "}"
                                    "QPushButton:hover {"
                                    "  background-color: %8;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "  background-color: %8;"
                                    "}"
                                    "QPushButton:disabled {"
                                    "  background-color: %9;"
                                    "  color: %10;"
                                    "}")
                                    .arg(colors.colorPrimary.name())
                                    .arg(colors.textPrimary.name())
                                    .arg(spacing.radiusMd)
                                    .arg(spacing.spacingLg)
                                    .arg(spacing.spacingXl * 2)
                                    .arg(spacing.fontWeightMedium)
                                    .arg(spacing.fontSizeMd)
                                    .arg(colors.colorPrimaryHover.name())
                                    .arg(colors.surfaceSecondary.name())
                                    .arg(colors.textDisabled.name());
    setStyleSheet(buttonStyle);
}

bool SendMessageButton::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QPushButton::event(event);
}

}  // namespace Sending
