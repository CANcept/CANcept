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

#pragma once

#include <memory>
#include <string>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

/**
 * @brief Leaf token referencing a named variable, keeping its value alive via shared ownership.
 */
class VariableToken final : public Token<TokenKind::Leaf>
{
   public:
    /**
     * @brief Constructs a variable token with shared ownership of the underlying value.
     */
    VariableToken(std::string name, std::shared_ptr<double> value)
        : m_name(std::move(name)), m_value(std::move(value))
    {
    }

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        return m_name;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    /**
     * @brief Collects this variable's name and raw pointer for exprtk symbol registration.
     */
    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        out.emplace_back(m_name, m_value.get());
    }

    /**
     * @brief Returns a raw pointer to the variable's current value.
     */
    [[nodiscard]] auto get() const -> double*
    {
        return m_value.get();
    }

   private:
    std::string m_name;
    std::shared_ptr<double> m_value;
};

}  // namespace Math
