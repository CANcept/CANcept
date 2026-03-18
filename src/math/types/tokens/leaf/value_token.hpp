#pragma once

#include <string>

#include "math/types/tokens/Token.hpp"

namespace Math
{

/**
 * @brief Leaf token holding a fixed numeric constant.
 */
class ValueToken final : public Token<TokenKind::Leaf>
{
public:
    explicit ValueToken(const double value) : m_value(value) {}

    std::string toExpression() const override
    {
        return std::to_string(m_value);
    }
    std::string label() const override
    {
        return std::to_string(m_value);
    }

    double value() const
    {
        return m_value;
    }

private:
    double m_value;
};

}  // namespace Math
