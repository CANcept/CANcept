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
#include <utility>
#include <vector>

namespace Math {

enum class TokenKind { Leaf, Internal };

class IExpressionVisitor;

template <TokenKind>
class Token;

struct TokenBase {
    virtual ~TokenBase() = default;
    [[nodiscard]] virtual auto toExpression() const -> std::string = 0;

    virtual void collectVariables(std::vector<std::pair<std::string, double*>>& out) const {}

    virtual void accept(IExpressionVisitor& visitor) const = 0;
};

template <TokenKind Kind>
class Token : public TokenBase
{
   public:
    [[nodiscard]] auto toExpression() const -> std::string override = 0;
};

template <>
class Token<TokenKind::Internal> : public TokenBase
{
   public:
    [[nodiscard]] auto toExpression() const -> std::string override = 0;

    [[nodiscard]] virtual auto expectedChildCount() const -> int = 0;

    void collectVariables(std::vector<std::pair<std::string, double*>>& out) const override
    {
        for (const auto& child : m_children)
        {
            if (child) child->collectVariables(out);
        }
    }

    void addChild(std::unique_ptr<TokenBase> child)
    {
        m_children.push_back(std::move(child));
    }

    void setChild(const int index, std::unique_ptr<TokenBase> child)
    {
        if (std::cmp_greater_equal(index, m_children.size()))
            m_children.resize(static_cast<std::size_t>(index) + 1);
        m_children[static_cast<std::size_t>(index)] = std::move(child);
    }

    void removeChild(const std::size_t index)
    {
        if (index < m_children.size()) m_children[index].reset();
    }

    [[nodiscard]] auto children() const -> const std::vector<std::unique_ptr<TokenBase>>&
    {
        return m_children;
    }

   protected:
    std::vector<std::unique_ptr<TokenBase>> m_children;
};

}  // namespace Math
