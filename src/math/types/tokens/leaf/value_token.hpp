#pragma once

#include <string>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

class ValueToken final : public Token<TokenKind::Leaf>
{
   public:
    explicit ValueToken(const double value) : m_value(value) {}

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        return std::to_string(m_value);
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    [[nodiscard]] auto value() const -> double
    {
        return m_value;
    }

   private:
    double m_value;
};

}  // namespace Math
