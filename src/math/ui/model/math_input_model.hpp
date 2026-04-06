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

#include <QObject>
#include <QString>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "math/types/tokens/token.hpp"
#include "math/types/variables/i_variable.hpp"

namespace Math {

class VariableRegistry;

struct SlotAddress {
    const Token<TokenKind::Internal>* parent = nullptr;
    int childIndex = -1;

    auto operator==(const SlotAddress& other) const -> bool = default;
};

/**
 * @brief Binds a single-char symbol to a variable instance in the global registry.
 */
struct VariableBinding {
    char symbol = '\0';
    int typeIndex = 0;
    IVariable* variable = nullptr;
};

class MathInputModel final : public QObject
{
    Q_OBJECT

   public:
    explicit MathInputModel(VariableRegistry& registry, QObject* parent = nullptr);
    ~MathInputModel() override;

    void addNode(std::unique_ptr<TokenBase> token, Token<TokenKind::Internal>* parent,
                 int childIndex = -1);
    void removeNode(const TokenBase* token);

    [[nodiscard]] auto root() const -> const TokenBase*;

    /**
     * @brief Creates a variable token for the given symbol character.
     *
     * Looks up the symbol in the current bindings and creates a VariableToken
     * with shared ownership of the variable's value.
     */
    [[nodiscard]] auto makeVariableToken(const std::string& symbolStr)
        -> std::unique_ptr<TokenBase>;

    /**
     * @brief Replaces the variable bindings, handling acquire/release with the registry.
     */
    void setVariableBindings(std::vector<VariableBinding> bindings);

    [[nodiscard]] auto variableBindings() const -> const std::vector<VariableBinding>&;

    void insertToken(std::unique_ptr<TokenBase> token);
    void activateSlot(const Token<TokenKind::Internal>* parent, int childIndex);
    void clearEditorState();
    void selectNextEmptySlot();
    void appendToTypeBuffer(const QString& text);
    void chopTypeBuffer();
    void commitTypeBuffer();

    [[nodiscard]] auto activeSlot() const -> const std::optional<SlotAddress>&;
    [[nodiscard]] auto typeBuffer() const -> const QString&;
    [[nodiscard]] auto isSlotActive() const -> bool;
    [[nodiscard]] auto isComplete() const -> bool;

   signals:
    void changed();
    void editorStateChanged();

   private:
    [[nodiscard]] auto findParent(const TokenBase* target,
                                  Token<TokenKind::Internal>* node) -> Token<TokenKind::Internal>*;
    [[nodiscard]] auto findNextEmptySlot(TokenBase* node) const -> std::optional<SlotAddress>;

    VariableRegistry& m_registry;
    std::unique_ptr<TokenBase> m_root;
    std::vector<VariableBinding> m_bindings;
    std::optional<SlotAddress> m_activeSlot;
    QString m_typeBuffer;
};

}  // namespace Math
