#pragma once

#include <array>
#include <cassert>
#include <format>
#include <string>
#include <string_view>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

enum class Function { Sin, Cos, Abs, Log, Sqrt };

class FunctionToken final : public Token<TokenKind::Internal>
{
   public:
    explicit FunctionToken(const Function function) : m_func(function) {}

    [[nodiscard]] auto function() const -> Function
    {
        return m_func;
    }

    [[nodiscard]] auto expectedChildCount() const -> int override
    {
        return CHILD_COUNT;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        assert(m_children.size() == CHILD_COUNT);
        return std::format(EXPRESSION_FORMAT, symbol(), m_children[INDEX]->toExpression());
    }

   private:
    static constexpr int CHILD_COUNT = 1;
    static constexpr std::size_t INDEX = 0;
    static constexpr std::string EXPRESSION_FORMAT = "{}({})";

    static constexpr std::array<std::string_view, 5> SYMBOLS = {"sin", "cos", "abs", "log", "sqrt"};

    [[nodiscard]] auto symbol() const -> std::string_view
    {
        return SYMBOLS[static_cast<std::size_t>(m_func)];
    }

    Function m_func;
};

}  // namespace Math
