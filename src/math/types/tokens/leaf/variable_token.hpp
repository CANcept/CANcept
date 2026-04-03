#pragma once

#include <memory>
#include <string>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

/**
 * @brief Leaf token referencing a named variable, keeping its value alive via shared ownership.
 */
class VariableToken final : public Token<TokenKind::Leaf>
{
   public:
    /**
     * @brief Constructs a variable token with shared ownership of the underlying value.
     */
    VariableToken(std::string name, std::shared_ptr<double> value)
        : m_name(std::move(name)), m_value(std::move(value))
    {
    }

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        return m_name;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    /**
     * @brief Collects this variable's name and raw pointer for exprtk symbol registration.
     */
    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        out.emplace_back(m_name, m_value.get());
    }

    /**
     * @brief Returns a raw pointer to the variable's current value.
     */
    [[nodiscard]] auto get() const -> double*
    {
        return m_value.get();
    }

   private:
    std::string m_name;
    std::shared_ptr<double> m_value;
};

}  // namespace Math
