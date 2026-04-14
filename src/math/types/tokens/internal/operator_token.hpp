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

#include <array>
#include <cassert>
#include <format>
#include <string>
#include <string_view>

#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/token.hpp"

namespace Math {

enum class Operation { Add, Sub, Mul, Div };

class OperatorToken final : public Token<TokenKind::Internal>
{
   public:
    explicit OperatorToken(const Operation operation) : m_op(operation) {}

    [[nodiscard]] auto operation() const -> Operation
    {
        return m_op;
    }

    [[nodiscard]] auto expectedChildCount() const -> int override
    {
        return CHILD_COUNT;
    }

    void accept(IExpressionVisitor& visitor) const override
    {
        visitor.visit(*this);
    }

    [[nodiscard]] auto toExpression() const -> std::string override
    {
        assert(m_children.size() == CHILD_COUNT);
        return std::format(EXPRESSION_FORMAT, m_children[LHS_INDEX]->toExpression(), symbol(),
                           m_children[RHS_INDEX]->toExpression());
    }

   private:
    static constexpr int CHILD_COUNT = 2;
    static constexpr std::size_t LHS_INDEX = 0;
    static constexpr std::size_t RHS_INDEX = 1;
    static constexpr std::string EXPRESSION_FORMAT = "({} {} {})";

    static constexpr std::array<std::string_view, 4> SYMBOLS = {"+", "-", "*", "/"};

    [[nodiscard]] auto symbol() const -> std::string_view
    {
        return SYMBOLS[static_cast<std::size_t>(m_op)];
    }

    Operation m_op;
};

}  // namespace Math
