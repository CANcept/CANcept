#include "styled_switch.hpp"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

StyledSwitch::StyledSwitch(QWidget* parent) : QAbstractButton(parent), m_hovered(false)
{
    setCheckable(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateThemeColors();
}

QSize StyledSwitch::sizeHint() const
{
    const auto& spacing = THEME.spacing();
    const int height = spacing.spacingLg * 1.7;
    const int width = height * 1.8;
    return QSize(width, height);
}

void StyledSwitch::updateThemeColors()
{
    const auto& colors = THEME.colors();

    m_trackColorOff = colors.surfacePrimary;
    m_trackColorOn = colors.surfacePrimary;
    m_thumbColor = colors.surfaceSecondary;
    m_hoverColor = colors.colorPrimaryHover;
}

void StyledSwitch::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int width = this->width();
    const int height = this->height();

    // Track
    const qreal trackHeight = height * 0.85;
    const qreal trackRadius = height / 2.0;
    const qreal thumbRadius = trackHeight * 0.4;
    const qreal trackY = (height - trackHeight) / 2.0;
    const QRectF trackRect(0, trackY, width, trackHeight);

    QColor trackColor;
    if (m_hovered)
    {
        trackColor = m_hoverColor;
    } else
    {
        trackColor = isChecked() ? m_trackColorOn : m_trackColorOff;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(trackRect, trackRadius, trackRadius);

    // Thumb
    const qreal thumbY = height / 2.0;
    const qreal padding = trackRadius * 0.5;
    qreal thumbX;

    if (isChecked())
    {
        thumbX = width - thumbRadius - padding;
    } else
    {
        thumbX = thumbRadius + padding;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_thumbColor);
    painter.drawEllipse(QPointF(thumbX, thumbY), thumbRadius, thumbRadius);
}

void StyledSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
        return;
    }
    QAbstractButton::mousePressEvent(event);
}

void StyledSwitch::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
    {
        setChecked(!isChecked());
        update();
        event->accept();
        return;
    }
    QAbstractButton::mouseReleaseEvent(event);
}

void StyledSwitch::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void StyledSwitch::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}

bool StyledSwitch::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        updateThemeColors();
        update();
        return true;
    }
    return QAbstractButton::event(event);
}

}  // namespace Core
