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

#include "math/ui/model/math_input_model.hpp"

#include <algorithm>
#include <utility>

#include "math/constants.hpp"
#include "math/service/variable_registry.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "math/ui/model/token_registry.hpp"

namespace Math {

MathInputModel::MathInputModel(VariableRegistry& registry, QObject* parent)
    : QObject(parent), m_registry(registry)
{
}

MathInputModel::~MathInputModel()
{
    for (const auto& binding : m_bindings)
    {
        if (binding.variable) m_registry.release(binding.variable->configKey());
    }
}

void MathInputModel::addNode(std::unique_ptr<TokenBase> token, Token<TokenKind::Internal>* parent,
                             const int childIndex)
{
    if (!parent)
    {
        m_root = std::move(token);
    } else if (childIndex >= 0)
    {
        parent->setChild(childIndex, std::move(token));
    } else
    {
        parent->addChild(std::move(token));
    }

    emit changed();
    selectNextEmptySlot();
}

void MathInputModel::removeNode(const TokenBase* token)
{
    if (!token) return;

    if (token == m_root.get())
    {
        m_root.reset();
        emit changed();
        selectNextEmptySlot();
        return;
    }

    auto* parent = findParent(token, dynamic_cast<Token<TokenKind::Internal>*>(m_root.get()));
    if (!parent) return;

    const auto& ch = parent->children();
    for (std::size_t i = 0; i < ch.size(); ++i)
    {
        if (ch[i].get() == token)
        {
            parent->removeChild(i);
            emit changed();
            selectNextEmptySlot();
            return;
        }
    }
}

auto MathInputModel::root() const -> const TokenBase*
{
    return m_root.get();
}

auto MathInputModel::makeVariableToken(const std::string& symbolStr) -> std::unique_ptr<TokenBase>
{
    if (symbolStr.size() != 1) return nullptr;

    const char sym = symbolStr[0];
    for (const auto& binding : m_bindings)
    {
        if (binding.symbol == sym && binding.variable)
        {
            return std::make_unique<VariableToken>(symbolStr, binding.variable->sharedPtr());
        }
    }
    return nullptr;
}

void MathInputModel::setVariableBindings(std::vector<VariableBinding> bindings)
{
    for (const auto& old : m_bindings)
    {
        if (old.variable) m_registry.release(old.variable->configKey());
    }

    m_bindings = std::move(bindings);
}

auto MathInputModel::variableBindings() const -> const std::vector<VariableBinding>&
{
    return m_bindings;
}

void MathInputModel::insertToken(std::unique_ptr<TokenBase> token)
{
    if (!m_root)
    {
        addNode(std::move(token), nullptr);
    } else if (m_activeSlot)
    {
        auto* parent = const_cast<Token<TokenKind::Internal>*>(m_activeSlot->parent);
        addNode(std::move(token), parent, m_activeSlot->childIndex);
    }
}

void MathInputModel::activateSlot(const Token<TokenKind::Internal>* parent, int childIndex)
{
    if (parent)
    {
        if (const int expected = parent->expectedChildCount(); childIndex >= expected)
        {
            childIndex = expected - 1;
        }
    }
    m_activeSlot = SlotAddress{.parent = parent, .childIndex = childIndex};
    m_typeBuffer.clear();
    emit editorStateChanged();
}

void MathInputModel::clearEditorState()
{
    m_activeSlot.reset();
    m_typeBuffer.clear();
    emit editorStateChanged();
}

void MathInputModel::selectNextEmptySlot()
{
    if (!m_root)
    {
        m_activeSlot = SlotAddress{.parent = nullptr, .childIndex = 0};
    } else
    {
        auto slot = findNextEmptySlot(m_root.get());
        m_activeSlot = slot.has_value() ? std::optional<SlotAddress>(*slot) : std::nullopt;
    }
    m_typeBuffer.clear();
    emit editorStateChanged();
}

void MathInputModel::appendToTypeBuffer(const QString& text)
{
    m_typeBuffer += text;
    emit editorStateChanged();
}

void MathInputModel::chopTypeBuffer()
{
    m_typeBuffer.chop(1);
    emit editorStateChanged();
}

void MathInputModel::commitTypeBuffer()
{
    if (!m_activeSlot || m_typeBuffer.isEmpty())
    {
        clearEditorState();
        return;
    }

    const QString text = m_typeBuffer;
    auto* parent = const_cast<Token<TokenKind::Internal>*>(m_activeSlot->parent);
    const int childIdx = m_activeSlot->childIndex;
    clearEditorState();

    if (text.startsWith('\\') && text.size() == 2)
    {
        if (const auto* entry = TokenRegistry::findByShortcut(text[1]))
        {
            addNode(entry->factory(), parent, childIdx);
            return;
        }
    } else if (text.size() == 1)
    {
        if (const QChar symbol = text[0];
            std::find(std::begin(Constants::WHITELIST_SHORTCUTS),
                      std::end(Constants::WHITELIST_SHORTCUTS),
                      symbol) != std::end(Constants::WHITELIST_SHORTCUTS))
        {
            if (const auto* entry = TokenRegistry::findByShortcut(symbol))
            {
                addNode(entry->factory(), parent, childIdx);
                return;
            }
        }
    }

    std::unique_ptr<TokenBase> token;
    if (text[0].isLetter())
    {
        token = makeVariableToken(text.toStdString());
    } else
    {
        bool ok = false;
        const double val = text.toDouble(&ok);
        if (ok)
        {
            token = std::make_unique<ValueToken>(val);
        }
    }

    if (token) addNode(std::move(token), parent, childIdx);
}

auto MathInputModel::activeSlot() const -> const std::optional<SlotAddress>&
{
    return m_activeSlot;
}

auto MathInputModel::typeBuffer() const -> const QString&
{
    return m_typeBuffer;
}

auto MathInputModel::isSlotActive() const -> bool
{
    return m_activeSlot.has_value();
}

auto MathInputModel::isComplete() const -> bool
{
    return m_root && !findNextEmptySlot(m_root.get());
}

auto MathInputModel::findParent(const TokenBase* target,
                                Token<TokenKind::Internal>* node) -> Token<TokenKind::Internal>*
{
    if (!node) return nullptr;

    for (const auto& child : node->children())
    {
        if (!child) continue;
        if (child.get() == target) return node;
        if (auto* inner = dynamic_cast<Token<TokenKind::Internal>*>(child.get()))
        {
            if (auto* found = findParent(target, inner)) return found;
        }
    }
    return nullptr;
}

auto MathInputModel::findNextEmptySlot(TokenBase* node) const -> std::optional<SlotAddress>
{
    const auto* internal = dynamic_cast<Token<TokenKind::Internal>*>(node);
    if (!internal)
    {
        return std::nullopt;
    }

    const auto& children = internal->children();
    const int expected = internal->expectedChildCount();

    for (int i = 0; i < expected; ++i)
    {
        if (std::cmp_less_equal(children.size(), i) || !children[static_cast<std::size_t>(i)])
        {
            return SlotAddress{.parent = internal, .childIndex = i};
        }

        if (const auto result = findNextEmptySlot(children[static_cast<std::size_t>(i)].get()))
        {
            return result;
        }
    }
    return std::nullopt;
}

}  // namespace Math
