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

enum class Function { Sin, Cos, Abs, Log, Sqrt };

class FunctionToken final : public Token<TokenKind::Internal>
{
   public:
    explicit FunctionToken(const Function function) : m_func(function) {}

    [[nodiscard]] auto function() const -> Function
    {
        return m_func;
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
        return std::format(EXPRESSION_FORMAT, symbol(), m_children[INDEX]->toExpression());
    }

   private:
    static constexpr int CHILD_COUNT = 1;
    static constexpr std::size_t INDEX = 0;
    static constexpr std::string EXPRESSION_FORMAT = "{}({})";

    static constexpr std::array<std::string_view, 5> SYMBOLS = {"sin", "cos", "abs", "log", "sqrt"};

    [[nodiscard]] auto symbol() const -> std::string_view
    {
        return SYMBOLS[static_cast<std::size_t>(m_func)];
    }

    Function m_func;
};

}  // namespace Math
