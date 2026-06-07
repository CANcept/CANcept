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

#include "math_input_status_indicator.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "math/constants.hpp"

namespace Math {

MathInputStatusIndicator::MathInputStatusIndicator(QWidget* parent)
    : QWidget(parent), m_spinTimer(new QTimer(this))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_spinTimer, &QTimer::timeout, this, [this] {
        m_spinAngle = (m_spinAngle + Constants::SPIN_STEP_DEG) % 360;
        update();
    });
    applyStyle();
}

auto MathInputStatusIndicator::sizeHint() const -> QSize
{
    const int size = THEME.spacing().spacingLg;
    return {size, size};
}

void MathInputStatusIndicator::startParsing()
{
    m_state = State::Parsing;
    m_spinAngle = 0;
    m_spinTimer->start(Constants::SPIN_INTERVAL_MS);
    update();
}

void MathInputStatusIndicator::setParseResult(const ValueFunction::ParseResult& result)
{
    m_spinTimer->stop();
    m_state = result.success ? State::Success : State::Error;
    m_errorText = result.success ? QString() : QString::fromStdString(result.error);
    setToolTip(m_errorText);
    update();
}

void MathInputStatusIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (m_state)
    {
        case State::Parsing:
            drawSpinner(painter);
            break;
        case State::Success:
            drawCheck(painter);
            break;
        case State::Error:
            drawCross(painter);
            break;
        case State::Idle:
            break;
    }
}

void MathInputStatusIndicator::drawSpinner(QPainter& painter) const
{
    painter.setPen(QPen(m_spinColor, Constants::STROKE_WIDTH, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    const QRectF r =
        QRectF(rect()).adjusted(Constants::INDICATOR_MARGIN, Constants::INDICATOR_MARGIN,
                                -Constants::INDICATOR_MARGIN, -Constants::INDICATOR_MARGIN);
    painter.drawArc(r, m_spinAngle * 16, -Constants::SPIN_SPAN_DEG * 16);
}

void MathInputStatusIndicator::drawCheck(QPainter& painter) const
{
    const auto r = QRectF(rect());
    painter.setPen(
        QPen(m_successColor, Constants::STROKE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(r.topLeft() + QPointF(r.width() * Constants::CHECK_RATIO_START_X,
                                           r.height() * Constants::CHECK_RATIO_START_Y),
                     r.topLeft() + QPointF(r.width() * Constants::CHECK_RATIO_MID_X,
                                           r.height() * Constants::CHECK_RATIO_MID_Y));
    painter.drawLine(r.topLeft() + QPointF(r.width() * Constants::CHECK_RATIO_MID_X,
                                           r.height() * Constants::CHECK_RATIO_MID_Y),
                     r.topLeft() + QPointF(r.width() * Constants::CHECK_RATIO_END_X,
                                           r.height() * Constants::CHECK_RATIO_END_Y));
}

void MathInputStatusIndicator::drawCross(QPainter& painter) const
{
    const QRectF r =
        QRectF(rect()).adjusted(Constants::INDICATOR_MARGIN, Constants::INDICATOR_MARGIN,
                                -Constants::INDICATOR_MARGIN, -Constants::INDICATOR_MARGIN);
    painter.setPen(QPen(m_errorColor, Constants::STROKE_WIDTH, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(r.topLeft(), r.bottomRight());
    painter.drawLine(r.topRight(), r.bottomLeft());
}

auto MathInputStatusIndicator::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        update();
        return true;
    }
    return QWidget::event(event);
}

void MathInputStatusIndicator::applyStyle()
{
    const auto& colors = THEME.colors();
    m_successColor = colors.statusSuccess;
    m_errorColor = colors.statusError;
    m_spinColor = colors.colorPrimary;
}

}  // namespace Math
