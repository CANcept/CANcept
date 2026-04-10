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

#include "math/ui/view/visitor/expression_measure_visitor.hpp"

#include <algorithm>
#include <utility>

#include "core/macro/theme.hpp"
#include "math/constants.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/model/token_registry.hpp"

namespace Math {
using namespace Constants;

ExpressionMeasureVisitor::ExpressionMeasureVisitor(const QFontMetrics& fm,
                                                   const MathInputModel* model)
    : m_fontMetric(fm), m_model(model)
{
}

auto ExpressionMeasureVisitor::measureToken(const TokenBase* token) -> QSizeF
{
    if (!token) return measureSlot(nullptr, 0);
    token->accept(*this);
    return m_size;
}

auto ExpressionMeasureVisitor::measureSlot(const Token<TokenKind::Internal>* parent,
                                           const int childIndex) const -> QSizeF
{
    const auto& spacing = THEME.spacing();
    if (const auto& slot = m_model->activeSlot(); slot && slot->parent == parent &&
                                                  slot->childIndex == childIndex &&
                                                  !m_model->typeBuffer().isEmpty())
    {
        const int width =
            m_fontMetric.horizontalAdvance(m_model->typeBuffer()) + spacing.radiusXs + 4;
        return {static_cast<qreal>(std::max(width, spacing.HeightXs)),
                static_cast<qreal>(spacing.HeightXs)};
    }
    return {static_cast<qreal>(spacing.HeightXs), static_cast<qreal>(spacing.HeightXs)};
}

void ExpressionMeasureVisitor::visit(const ValueToken& token)
{
    const auto& spacing = THEME.spacing();
    const int width =
        m_fontMetric.horizontalAdvance(QString::number(token.value())) + 2 * spacing.spacingXs;
    const int height = m_fontMetric.height() + 2 * spacing.spacingXs * PAD_VERTICAL_FACTOR;
    m_size = {static_cast<double>(width), static_cast<double>(std::max(height, spacing.HeightXs))};
}

void ExpressionMeasureVisitor::visit(const VariableToken& token)
{
    const auto& spacing = THEME.spacing();
    const int width = m_fontMetric.horizontalAdvance(QString::fromStdString(token.toExpression())) +
                      2 * spacing.spacingXs;
    const int height = m_fontMetric.height() + 2 * spacing.spacingXs * PAD_VERTICAL_FACTOR;
    m_size = {static_cast<double>(std::max(width, spacing.HeightXs)),
              static_cast<double>(std::max(height, spacing.HeightXs))};
}

void ExpressionMeasureVisitor::visit(const OperatorToken& token)
{
    const auto& spacing = THEME.spacing();
    if (token.operation() == Operation::Div)
    {
        const QSizeF num = childSize(token, 0);
        const QSizeF den = childSize(token, 1);
        const double fracW = std::max(num.width(), den.width()) + 2 * spacing.radiusXs;
        m_size = {fracW, num.height() + spacing.spacingXs + 2.0 + spacing.spacingXs + den.height()};
    } else
    {
        const QString sym = TokenRegistry::operatorSymbol(token.operation());
        const QSizeF lhs = childSize(token, 0);
        const QSizeF rhs = childSize(token, 1);
        const int opW = m_fontMetric.horizontalAdvance(sym) + 2 * spacing.radiusXs;
        const int opH = m_fontMetric.height() + 2 * spacing.spacingXs * PAD_VERTICAL_FACTOR;
        const int bracketW = m_fontMetric.horizontalAdvance('(') + spacing.radiusXs;
        const auto needsBrackets = [&](const int index) -> bool
        {
            const auto& children = token.children();
            if (std::cmp_less_equal(children.size(), index) || !children[index])
            {
                return false;
            }
            const auto* child = dynamic_cast<const OperatorToken*>(children[index].get());
            return child != nullptr && child->operation() != Operation::Div;
        };

        const double lhsExtra = needsBrackets(0) ? bracketW * 2 : 0;
        const double rhsExtra = needsBrackets(1) ? bracketW * 2 : 0;
        m_size = {lhsExtra + lhs.width() + spacing.spacingXs + opW + spacing.spacingXs + rhsExtra + rhs.width(),
                  std::max({lhs.height(), static_cast<double>(opH), rhs.height()})};
    }
}

void ExpressionMeasureVisitor::visit(const FunctionToken& token)
{
    const auto& spacing = THEME.spacing();
    const QSizeF arg = childSize(token, 0);
    switch (token.function())
    {
        case Function::Sqrt: {
            m_size = {spacing.spacingMd + arg.width() + spacing.radiusXs, arg.height() + 4.0};
            break;
        }
        case Function::Abs: {
            const int barPad = static_cast<int>(spacing.spacingXs * BAR_PAD_FACTOR);
            const int barW = m_fontMetric.horizontalAdvance("|");
            m_size = {barW + barPad + arg.width() + barPad + barW,
                      std::max(arg.height(), m_fontMetric.height() +
                                                 2 * spacing.spacingXs * PAD_VERTICAL_FACTOR)};
            break;
        }
        default: {
            const QString name = TokenRegistry::functionLabel(token.function());
            const int nameWidth = m_fontMetric.horizontalAdvance(name);
            const int nameHeight =
                m_fontMetric.height() + 2 * spacing.spacingXs * PAD_VERTICAL_FACTOR;
            const int parenthesesWidth =
                m_fontMetric.horizontalAdvance("(") + m_fontMetric.horizontalAdvance(")");
            m_size = {static_cast<double>(nameWidth + parenthesesWidth) + arg.width(),
                      std::max(static_cast<double>(nameHeight), arg.height())};
            break;
        }
    }
}

auto ExpressionMeasureVisitor::childSize(const Token<TokenKind::Internal>& parent,
                                         const int index) -> QSizeF
{
    if (const auto& children = parent.children();
        static_cast<int>(children.size()) > index && children[index])
    {
        return measureToken(children[index].get());
    }
    return measureSlot(&parent, index);
}

}  // namespace Math
