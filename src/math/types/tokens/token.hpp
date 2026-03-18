#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Math
{

/**
 * @brief Defines whether a token is a terminal node or an operator with children.
 */
enum class TokenKind
{
    Leaf,
    Internal
};

/**
 * @brief Type-erased base for storing mixed token kinds in a single collection.
 */
struct TokenBase
{
    virtual ~TokenBase() = default;
    virtual std::string toExpression() const = 0;
    virtual std::string label() const = 0;

    /** @brief Contributes name/ptr pairs of any variable tokens in this subtree. */
    virtual void collectVariables(std::vector<std::pair<std::string, double*>>& out) const {}
};

/**
 * @brief Terminal token with no children, represents a single atomic operand.
 */
template<TokenKind Kind>
class Token : public TokenBase
{
public:
    std::string toExpression() const override = 0;
    std::string label() const override = 0;
};

/**
 * @brief Branch token that owns child tokens and composes them into a sub-expression.
 */
template<>
class Token<TokenKind::Internal> : public TokenBase
{
public:
    std::string toExpression() const override = 0;
    std::string label() const override = 0;

    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        for (const auto& child : m_children)
            child->collectVariables(out);
    }

    void addChild(std::unique_ptr<TokenBase> child)
    {
        m_children.push_back(std::move(child));
    }

    const std::vector<std::unique_ptr<TokenBase>>& children() const
    {
        return m_children;
    }

protected:
    std::vector<std::unique_ptr<TokenBase>> m_children;
};

}  // namespace Math
