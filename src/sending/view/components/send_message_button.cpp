#include "send_message_button.hpp"

#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "sending/constants.hpp"

namespace Sending {

SendMessageButton::SendMessageButton(QWidget* parent) : QPushButton(parent), m_isSending(false)
{
    setText(Constants::SEND_BUTTON_TEXT);
    setMinimumHeight(Constants::SEND_BUTTON_MIN_HEIGHT);
    setMinimumWidth(Constants::SEND_BUTTON_MIN_WIDTH);

    updateAppearance();
}

void SendMessageButton::setSendingState(bool sending)
{
    if (m_isSending == sending)
    {
        return;
    }

    m_isSending = sending;
    updateAppearance();
}

void SendMessageButton::updateAppearance()
{
    if (m_isSending)
    {
        setText("Stop");
    } else
    {
        setText(Constants::SEND_BUTTON_TEXT);
    }

    applyStyle();
}

void SendMessageButton::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    QString iconPath;
    QColor iconColor;

    if (m_isSending)
    {
        iconPath = Constants::STOP_SENDING_ICON_PATH;
        iconColor = colors.textOnPrimary;
    } else
    {
        iconPath = Constants::SEND_BUTTON_ICON_PATH;
        iconColor = colors.textPrimary;
    }

    QPixmap pixmap(iconPath);
    if (!pixmap.isNull())
    {
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), iconColor);
        painter.end();
        setIcon(QIcon(pixmap));
    }

    // Set overall styles to align with the project
    const int iconSize = spacing.spacingLg;
    setIconSize(QSize(iconSize, iconSize));

    QColor bgColor, bgHoverColor, textColor;

    if (m_isSending)
    {
        bgColor = colors.statusError;
        bgHoverColor = colors.statusError.darker(110);
        textColor = colors.textOnPrimary;
    } else
    {
        bgColor = colors.colorPrimary;
        bgHoverColor = colors.colorPrimaryHover;
        textColor = colors.textPrimary;
    }

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
                                    .arg(bgColor.name())
                                    .arg(textColor.name())
                                    .arg(spacing.radiusMd)
                                    .arg(spacing.spacingLg)
                                    .arg(spacing.spacingXl * 2)
                                    .arg(spacing.fontWeightMedium)
                                    .arg(spacing.fontSizeMd)
                                    .arg(bgHoverColor.name())
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
