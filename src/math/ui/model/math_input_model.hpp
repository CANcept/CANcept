#pragma once

#include <QObject>
#include <QString>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "math/types/tokens/token.hpp"

namespace Math {

struct SlotAddress {
    const Token<TokenKind::Internal>* parent = nullptr;
    int childIndex = -1;

    auto operator==(const SlotAddress& other) const -> bool = default;
};

class MathInputModel final : public QObject
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs an empty model with no expression tree.
     */
    explicit MathInputModel(QObject* parent = nullptr);

    /**
     * @brief Inserts a token as a child of the given parent at the specified index.
     */
    void addNode(std::unique_ptr<TokenBase> token, Token<TokenKind::Internal>* parent,
                 int childIndex = -1);

    /**
     * @brief Removes the given token from the tree and resets its slot to empty.
     */
    void removeNode(const TokenBase* token);

    /**
     * @brief Returns the root token of the expression tree, or nullptr if empty.
     */
    [[nodiscard]] auto root() const -> const TokenBase*;

    /**
     * @brief Creates a variable token whose value is backed by this model's variable store.
     */
    [[nodiscard]] auto makeVariableToken(const std::string& name) -> std::unique_ptr<TokenBase>;

    /**
     * @brief Inserts a token into the currently active slot, or as root if the tree is empty.
     */
    void insertToken(std::unique_ptr<TokenBase> token);

    /**
     * @brief Sets the active editing slot to the given parent and child index.
     */
    void activateSlot(const Token<TokenKind::Internal>* parent, int childIndex);

    /**
     * @brief Clears the active slot and type buffer, leaving no slot selected.
     */
    void clearEditorState();

    /**
     * @brief Advances the active slot to the next unfilled child in the tree.
     */
    void selectNextEmptySlot();

    /**
     * @brief Appends text to the inline type buffer of the active slot.
     */
    void appendToTypeBuffer(const QString& text);

    /**
     * @brief Removes the last character from the type buffer.
     */
    void chopTypeBuffer();

    /**
     * @brief Parses the type buffer and inserts the resulting token into the active slot.
     */
    void commitTypeBuffer();

    /**
     * @brief Returns the currently active slot address, if any.
     */
    [[nodiscard]] auto activeSlot() const -> const std::optional<SlotAddress>&;

    /**
     * @brief Returns the current inline type buffer contents.
     */
    [[nodiscard]] auto typeBuffer() const -> const QString&;

    /**
     * @brief Returns whether any slot is currently active for editing.
     */
    [[nodiscard]] auto isSlotActive() const -> bool;

    /**
     * @brief Returns whether the expression tree is fully filled with no empty slots.
     */
    [[nodiscard]] auto isComplete() const -> bool;

   signals:
    void changed();
    void editorStateChanged();

   private:
    [[nodiscard]] auto findParent(const TokenBase* target,
                                  Token<TokenKind::Internal>* node) -> Token<TokenKind::Internal>*;
    [[nodiscard]] auto findNextEmptySlot(TokenBase* node) const -> std::optional<SlotAddress>;

    std::unique_ptr<TokenBase> m_root;
    std::map<std::string, double> m_variableValues;
    std::optional<SlotAddress> m_activeSlot;
    QString m_typeBuffer;
};

}  // namespace Math
