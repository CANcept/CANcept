#pragma once

#include <string>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

class VariableToken final : public Token<TokenKind::Leaf>
{
   public:
    VariableToken(std::string name, double* ptr) : m_name(std::move(name)), m_ptr(ptr) {}

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        return m_name;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        out.emplace_back(m_name, m_ptr);
    }

    [[nodiscard]] auto get() const -> double*
    {
        return m_ptr;
    }

   private:
    std::string m_name;
    double* m_ptr;
};

}  // namespace Math
