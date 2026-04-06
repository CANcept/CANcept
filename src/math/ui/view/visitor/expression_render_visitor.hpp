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

#pragma once

#include <QList>
#include <QPainter>
#include <QPoint>
#include <QSizeF>

#include "expression_measure_visitor.hpp"
#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "math/ui/view/components/math_expression_widget.hpp"

namespace Math {

class MathInputModel;

/**
 * @brief Paints the expression tree onto a QPainter surface and records hit regions for
 * interaction.
 */
class ExpressionRenderVisitor final : public IExpressionVisitor
{
   public:
    /**
     * @brief Constructs a render visitor with painting context, model state, and interaction data.
     */
    ExpressionRenderVisitor(QPainter& painter, const QFontMetrics& fontMetrics,
                            MathInputModel* model, QList<HitRegion>& hitRegions, QPoint mousePos,
                            bool cursorVisible);

    /**
     * @brief Recursively paints a token subtree at the given origin, or an empty slot if null.
     */
    void renderToken(const TokenBase* token, QPointF origin);

    void visit(const ValueToken& token) override;
    void visit(const VariableToken& token) override;
    void visit(const OperatorToken& token) override;
    void visit(const FunctionToken& token) override;

   private:
    void renderFraction(const OperatorToken& token);
    void renderInlineOp(const OperatorToken& token);
    void renderFunction(const FunctionToken& token);
    void renderRadical(const FunctionToken& token);
    void renderAbsBars(const FunctionToken& token);

    auto childSize(const Token<TokenKind::Internal>& parent, int index) -> QSizeF;
    void renderChild(const Token<TokenKind::Internal>* parent, int index, QPointF pos);

    void paintSlot(const QRectF& rect, const Token<TokenKind::Internal>* parent,
                   int childIndex) const;
    void drawEmptySlot(const QRectF& rect) const;
    void drawActiveSlot(const QRectF& rect) const;

    QPainter& m_painter;
    const QFontMetrics& m_fm;
    MathInputModel* m_model;
    QList<HitRegion>& m_hitRegions;
    QPoint m_mousePos;
    bool m_cursorVisible;
    ExpressionMeasureVisitor m_measurer;
    QPointF m_origin;
};

}  // namespace Math
