#pragma once

namespace Math {

class ValueToken;
class VariableToken;
class OperatorToken;
class FunctionToken;

/**
 * @brief Interface for visitor pattern traversal of the expression token tree.
 */
class IExpressionVisitor
{
   public:
    virtual ~IExpressionVisitor() = default;

    /**
     * @brief Processes a numeric literal token.
     */
    virtual void visit(const ValueToken& token) = 0;

    /**
     * @brief Processes a variable reference token.
     */
    virtual void visit(const VariableToken& token) = 0;

    /**
     * @brief Processes a operator token.
     */
    virtual void visit(const OperatorToken& token) = 0;

    /**
     * @brief Processes a function-call token.
     */
    virtual void visit(const FunctionToken& token) = 0;
};

}  // namespace Math
