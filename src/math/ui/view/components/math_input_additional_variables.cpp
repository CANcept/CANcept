
#include "math_input_additional_variables.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Math {

MathInputAdditionalVariables::MathInputAdditionalVariables(const QString& iconPath, QWidget* parent)
    : MathInputButton(iconPath, parent), m_outlineColor(THEME.colors().colorPrimary)
{
}

auto MathInputAdditionalVariables::sizeHint() const -> QSize
{
    const int size = THEME.spacing().spacingLg;
    return {size, size};
}

void MathInputAdditionalVariables::paintEvent(QPaintEvent* event)
{
    MathInputButton::paintEvent(event);

    if (underMouse())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(m_outlineColor, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), THEME.spacing().radiusXs,
                                THEME.spacing().radiusXs);
    }
}

auto MathInputAdditionalVariables::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        m_outlineColor = THEME.colors().colorPrimary;
    }
    return MathInputButton::event(event);
}

}  // namespace Math
