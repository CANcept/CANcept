#include "math/service/value_function.hpp"

namespace Math
{

ValueFunction::ValueFunction(std::unique_ptr<TokenBase> root)
    : m_root(std::move(root))
{
}

ValueFunction::ParseResult ValueFunction::parse()
{
    m_symbolTable = {};
    m_expression = {};
    m_expressionStr = m_root->toExpression();
    m_lastResult = {};

    m_symbolTable.add_constants();

    std::vector<std::pair<std::string, double*>> variables;
    m_root->collectVariables(variables);

    for (auto& [name, ptr] : variables)
        m_symbolTable.add_variable(name, *ptr);

    m_expression.register_symbol_table(m_symbolTable);

    if (exprtk::parser<double> parser; !parser.compile(m_expressionStr, m_expression))
    {
        m_lastResult = {false, parser.error()};
        return m_lastResult;
    }

    m_lastResult = {true, {}};
    return m_lastResult;
}

double ValueFunction::evaluate() const { return m_expression.value(); }
bool ValueFunction::isParsed() const { return m_lastResult.success; }
const ValueFunction::ParseResult& ValueFunction::lastResult() const { return m_lastResult; }
std::string_view ValueFunction::expression() const { return m_expressionStr; }

}  // namespace Math
