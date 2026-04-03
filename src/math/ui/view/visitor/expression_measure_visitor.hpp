#pragma once

#include <QFontMetrics>
#include <QSizeF>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/internal/operator_token.hpp"

namespace Math {

class MathInputModel;

/**
 * @brief Computes bounding-box sizes for each node in the expression tree.
 */
class ExpressionMeasureVisitor final : public IExpressionVisitor
{
   public:
    /**
     * @brief Constructs a measure visitor bound to the given font metrics and model state.
     */
    ExpressionMeasureVisitor(const QFontMetrics& fm, const MathInputModel* model);

    /**
     * @brief Returns the pixel size of a token subtree, or of a root-level empty slot if null.
     */
    auto measureToken(const TokenBase* token) -> QSizeF;

    /**
     * @brief Returns the pixel size of an unfilled child slot, accounting for active typing.
     */
    auto measureSlot(const Token<TokenKind::Internal>* parent, int childIndex) const -> QSizeF;

    void visit(const ValueToken& token) override;
    void visit(const VariableToken& token) override;
    void visit(const OperatorToken& token) override;
    void visit(const FunctionToken& token) override;

   private:
    auto childSize(const Token<TokenKind::Internal>& parent, int index) -> QSizeF;

    QSizeF m_size;
    const QFontMetrics& m_fontMetric;
    const MathInputModel* m_model;
};

}  // namespace Math