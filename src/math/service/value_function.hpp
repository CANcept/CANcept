#pragma once

#include <exprtk.hpp>
#include <string>

#include "math/types/tokens/token.hpp"

namespace Math {

/**
 * @brief Parses a token tree into a compiled exprtk expression and evaluates it.
 */
class ValueFunction
{
   public:
    /**
     * @brief Result of a parse attempt, stored internally and accessible after the fact.
     */
    struct ParseResult {
        bool success = false;
        std::string error;
    };

    explicit ValueFunction(const TokenBase* root);

    ValueFunction(const ValueFunction&) = delete;
    auto operator=(const ValueFunction&) -> ValueFunction& = delete;
    ValueFunction(ValueFunction&&) = default;
    auto operator=(ValueFunction&&) -> ValueFunction& = default;

    /**
     * @brief Walks the token tree to collect variables, builds the expression string, compiles.
     */
    auto parse() -> ParseResult;

    /**
     * @brief Evaluates the compiled expression based on the current state of the variable pointers.
     */
    [[nodiscard]] auto evaluate() const -> double;

    [[nodiscard]] auto isParsed() const -> bool;
    [[nodiscard]] auto lastResult() const -> const ParseResult&;
    [[nodiscard]] auto expression() const -> std::string_view;

   private:
    const TokenBase* m_root = nullptr;
    exprtk::symbol_table<double> m_symbolTable;
    exprtk::expression<double> m_expression;
    std::string m_expressionStr;
    ParseResult m_lastResult;
};

}  // namespace Math
