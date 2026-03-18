#pragma once

#include <string>

#include "math/types/tokens/Token.hpp"

namespace Math
{

/**
 * @brief Leaf token bound to an external double pointer registered at parse time.
 */
class VariableToken final : public Token<TokenKind::Leaf>
{
public:
    VariableToken(std::string name, double* ptr) : m_name(std::move(name)), m_ptr(ptr) {}

    std::string toExpression() const override
    {
        return m_name;
    }
    std::string label() const override
    {
        return m_name;
    }

    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        out.emplace_back(m_name, m_ptr);
    }

    double* get() const
    {
        return m_ptr;
    }

private:
    std::string m_name;
    double* m_ptr;
};

}  // namespace Math
