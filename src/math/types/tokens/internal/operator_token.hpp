#pragma once

#include <array>
#include <cassert>
#include <format>
#include <string_view>
#include <string>

#include "math/types/tokens/Token.hpp"

namespace Math
{

/**
 * @brief Arithmetic operation applied to exactly two child tokens.
 */
enum class Operation
{
    Add,
    Sub,
    Mul,
    Div
};

/**
 * @brief Combines two child tokens with an arithmetic operator to form a sub-expression.
 */
class OperatorToken final : public Token<TokenKind::Internal>
{
public:
    explicit OperatorToken(const Operation operation) : m_op(operation) {}

    std::string toExpression() const override
    {
        assert(m_children.size() == CHILD_COUNT);
        return std::format(EXPRESSION_FORMAT,
                           m_children[LHS_INDEX]->toExpression(),
                           symbol(),
                           m_children[RHS_INDEX]->toExpression());
    }

    std::string label() const override
    {
        return std::string(symbol());
    }

private:
    static constexpr std::size_t CHILD_COUNT = 2;
    static constexpr std::size_t LHS_INDEX = 0;
    static constexpr std::size_t RHS_INDEX = 1;
    static constexpr std::string EXPRESSION_FORMAT = "({} {} {})";

    static constexpr std::array<std::string_view, 4> SYMBOLS = {"+", "-", "*", "/"};

    std::string_view symbol() const
    {
        return SYMBOLS[static_cast<std::size_t>(m_op)];
    }

    Operation m_op;
};

}  // namespace Math
