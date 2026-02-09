#include "styled_line_edit.hpp"

#include <QPaintEvent>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

StyledLineEdit::StyledLineEdit(QWidget* parent) : QLineEdit(parent)
{
    setFrame(false);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_Hover, true);
    applyStyle();
}

StyledLineEdit::StyledLineEdit(const QString& text, QWidget* parent) : QLineEdit(text, parent)
{
    setFrame(false);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_Hover, true);
    applyStyle();
}

void StyledLineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();
    QColor borderColor = colors.borderSubtle;
    if (hasFocus())
    {
        borderColor = colors.colorPrimary;
    } else if (underMouse())
    {
        borderColor = colors.borderStrong;
    }

    // Draw the border
    const auto penWidth = static_cast<float>(spacing.borderThick);
    const float offset = penWidth / 2.0f;
    const QRectF borderRect = QRectF(rect()).adjusted(offset, offset, -offset, -offset);

    const QPen pen(borderColor, penWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.drawRoundedRect(borderRect, spacing.radiusSm, spacing.radiusSm);
}

void StyledLineEdit::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString style = QString(
                              "QLineEdit {"
                              "  background-color: %1;"
                              "  color: %2;"
                              "  border: none;"
                              "  border-radius: %3px;"
                              "  padding: %4px %5px;"
                              "  font-size: %6px;"
                              "  background-clip: padding;"
                              "  outline: none;"
                              "}"
                              "QLineEdit::placeholder {"
                              "  font-style: normal;"
                              "}"
                              "QLineEdit:disabled {"
                              "  background-color: %7;"
                              "}")
                              .arg(colors.surfaceMain.name(), colors.textSecondary.name())
                              .arg(spacing.radiusSm)
                              .arg(spacing.spacingLg)
                              .arg(spacing.spacingXl)
                              .arg(spacing.fontSizeSm)
                              .arg(colors.surfaceSecondary.name());

    setStyleSheet(style);
    updateGeometry();
}

bool StyledLineEdit::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QLineEdit::event(event);
}

}  // namespace Core