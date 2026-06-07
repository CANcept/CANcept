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

#include <string>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

class ValueToken final : public Token<TokenKind::Leaf>
{
   public:
    explicit ValueToken(const double value) : m_value(value) {}

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.17g", m_value);
        return buf;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    [[nodiscard]] auto value() const -> double
    {
        return m_value;
    }

   private:
    double m_value;
};

}  // namespace Math
