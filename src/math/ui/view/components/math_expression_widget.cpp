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

#include "math/ui/view/components/math_expression_widget.hpp"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "math/constants.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/view/visitor/expression_measure_visitor.hpp"
#include "math/ui/view/visitor/expression_render_visitor.hpp"

namespace Math {

MathExpressionWidget::MathExpressionWidget(MathInputModel* model, QWidget* parent)
    : QWidget(parent), m_model(model)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_cursorTimer.setInterval(500);
    connect(&m_cursorTimer, &QTimer::timeout, this, [this] {
        m_cursorVisible = !m_cursorVisible;
        update();
    });

    connect(m_model, &MathInputModel::changed, this, [this] {
        m_hitRegions.clear();
        updateGeometry();
        update();
    });
    connect(m_model, &MathInputModel::editorStateChanged, this, [this] {
        if (m_model->isSlotActive())
        {
            setFocus();
            resetCursorBlink();
        } else
        {
            m_cursorTimer.stop();
        }
        updateGeometry();
        update();
    });

    m_model->clearEditorState();
}

void MathExpressionWidget::paintEvent(QPaintEvent*)
{
    m_hitRegions.clear();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QFontMetrics fm(font());

    ExpressionRenderVisitor renderer(painter, fm, m_model, m_hitRegions, m_mousePos,
                                     m_cursorVisible);
    const int margin = static_cast<int>(THEME.spacing().spacingSm * Constants::MARGIN_FACTOR);
    renderer.renderToken(m_model->root(), QPointF(margin, margin));
}

void MathExpressionWidget::mousePressEvent(QMouseEvent* event)
{
    for (const auto& [rect, token, slotParent, slotChildIndex] : m_hitRegions)
    {
        if (!rect.contains(event->pos())) continue;

        if (!token)
        {
            m_model->activateSlot(slotParent, slotChildIndex);
        } else
        {
            m_model->removeNode(token);
        }
        return;
    }
    QWidget::mousePressEvent(event);
}

void MathExpressionWidget::keyPressEvent(QKeyEvent* event)
{
    if (m_model->isSlotActive())
    {
        switch (event->key())
        {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                m_model->commitTypeBuffer();
                return;

            case Qt::Key_Backspace:
                if (m_model->typeBuffer().isEmpty())
                    m_model->clearEditorState();
                else
                    m_model->chopTypeBuffer();
                resetCursorBlink();
                return;

            case Qt::Key_Escape:
                m_model->clearEditorState();
                return;

            default: {
                if (const QString text = event->text();
                    !text.isEmpty() &&
                    (text[0].isLetterOrNumber() || text[0] == '.' || text[0] == '\\' ||
                     isWhitelisted(text[0]) || (text.size() > 1 && isWhitelisted(text[1]))))
                {
                    m_model->appendToTypeBuffer(text);
                    resetCursorBlink();
                }
            }
        }
    }
    QWidget::keyPressEvent(event);
}

auto MathExpressionWidget::isWhitelisted(const QChar symbol) -> bool
{
    for (const char shortcut : Constants::WHITELIST_SHORTCUTS)
    {
        if (symbol == shortcut) return true;
    }

    return false;
}

void MathExpressionWidget::focusOutEvent(QFocusEvent* event)
{
    if (!m_model->typeBuffer().isEmpty()) m_model->commitTypeBuffer();
    QWidget::focusOutEvent(event);
}

void MathExpressionWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_mousePos = event->pos();
    update();
}

void MathExpressionWidget::leaveEvent(QEvent* event)
{
    m_mousePos = {-1, -1};
    update();
    QWidget::leaveEvent(event);
}

auto MathExpressionWidget::sizeHint() const -> QSize
{
    const QFontMetrics fontMetrics(font());
    ExpressionMeasureVisitor measurer(fontMetrics, m_model);
    const int margin = static_cast<int>(THEME.spacing().spacingSm * Constants::MARGIN_FACTOR);
    const QSizeF s = measurer.measureToken(m_model->root());
    return {static_cast<int>(s.width()) + 2 * margin, static_cast<int>(s.height()) + 2 * margin};
}

void MathExpressionWidget::resetCursorBlink()
{
    m_cursorVisible = true;
    m_cursorTimer.start();
    update();
}

}  // namespace Math
