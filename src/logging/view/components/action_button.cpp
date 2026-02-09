#include "action_button.hpp"

#include <QIcon>
#include <QStyle>

#include "core/macro/theme.hpp"

namespace Logging {

ActionButton::ActionButton(QWidget* parent) : QPushButton(parent)
{
    const auto& spacing = THEME.spacing();
    setIconSize(QSize(20, 20));
    setFixedSize(150, 75);
    applyStyle();
    setRecordingState(false);
}

void ActionButton::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString buttonStyle = QString(
                                    "QPushButton {"
                                    "   border: none;"
                                    "   border-radius: 30px;"
                                    "   font-family: 'Roboto';"
                                    "   font-size: 22px;"
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
                                    .arg(colors.statusErrorHover.name());
    setStyleSheet(buttonStyle);
}

void ActionButton::setRecordingState(bool isRecording)
{
    m_isRecording = isRecording;

    if (isRecording)
    {
        // Recording state - Red Stop button
        setProperty("recording", true);
        setText(" Stop");
        setIcon(QIcon(":/assets/icon/stop_logging.svg"));
    } else
    {
        // Idle state - Start button
        setProperty("recording", false);
        setText(" Start");
        setIcon(QIcon(":/assets/icon/logging_start.svg"));
    }

    // Force style refresh
    style()->unpolish(this);
    style()->polish(this);
}

}  // namespace Logging
