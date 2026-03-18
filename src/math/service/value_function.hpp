#pragma once

#include <memory>
#include <string>

#include <exprtk.hpp>

#include "math/types/tokens/token.hpp"

namespace Math
{

/**
 * @brief Parses a token tree into a compiled exprtk expression and evaluates it.
 */
class ValueFunction
{
public:
    /**
     * @brief Result of a parse attempt, stored internally and accessible after the fact.
     */
    struct ParseResult
    {
        bool success = false;
        std::string error;
    };

    explicit ValueFunction(std::unique_ptr<TokenBase> root);

    ValueFunction(const ValueFunction&) = delete;
    ValueFunction& operator=(const ValueFunction&) = delete;
    ValueFunction(ValueFunction&&) = default;
    ValueFunction& operator=(ValueFunction&&) = default;

    /**
     * @brief Walks the token tree to collect variables, builds the expression string, compiles.
     */
    ParseResult parse();

    /**
     * @brief Evaluates the compiled expression based on the current state of the variable pointers.
     */
    double evaluate() const;

    bool isParsed() const;
    const ParseResult& lastResult() const;
    std::string_view expression() const;

private:
    std::unique_ptr<TokenBase> m_root;
    exprtk::symbol_table<double> m_symbolTable;
    exprtk::expression<double> m_expression;
    std::string m_expressionStr;
    ParseResult m_lastResult;
};

}  // namespace Math
