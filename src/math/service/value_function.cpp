/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "math/service/value_function.hpp"

#include <iostream>

namespace Math {

ValueFunction::ValueFunction(const TokenBase* root) : m_root(root) {}

auto ValueFunction::parse() -> ParseResult
{
    m_symbolTable.clear();
    m_expression = {};
    m_expressionStr = m_root->toExpression();
    std::cout << m_expressionStr << std::endl;
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
