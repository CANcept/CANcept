#include "math/service/value_function.hpp"

namespace Math {

ValueFunction::ValueFunction(const TokenBase* root) : m_root(root) {}

auto ValueFunction::parse() -> ParseResult
{
    m_symbolTable.clear();
    m_expression = {};
    m_expressionStr = m_root->toExpression();
    m_lastResult = {};

    m_symbolTable.add_constants();

    std::vector<std::pair<std::string, double*>> variables;
    m_root->collectVariables(variables);

    for (auto& [name, ptr] : variables) m_symbolTable.add_variable(name, *ptr);

    m_expression.register_symbol_table(m_symbolTable);

    if (exprtk::parser<double> parser; !parser.compile(m_expressionStr, m_expression))
    {
        m_lastResult = {.success = false, .error = parser.error()};
        return m_lastResult;
    }

    m_lastResult = {.success = true, .error = {}};
    return m_lastResult;
}

auto ValueFunction::evaluate() const -> double
{
    return m_expression.value();
}
auto ValueFunction::isParsed() const -> bool
{
    return m_lastResult.success;
}
auto ValueFunction::lastResult() const -> const ParseResult&
{
    return m_lastResult;
}
auto ValueFunction::expression() const -> std::string_view
{
    return m_expressionStr;
}

}  // namespace Math
