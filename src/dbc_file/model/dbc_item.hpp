//
// Created by Adrian Rupp on 24.12.25.
//
#pragma once

#include <QVariant>
#include <memory>
#include <vector>

// Ensure this path is correct in your project structure
#include "core/enum/dbc_itemtype.hpp"

namespace DbcFile {

/**
 * @brief Represents a single node in the internal DBC tree structure.
 *
 * This class serves as the generic container for the DbcModel. It holds the data
 * for all types of nodes (Root, ECU, Message, Signal) and manages the
 * parent-child relationships required by QAbstractItemModel.
 *
 * Data Storage:
 * The member `m_data` holds a list of QVariants corresponding to the columns/attributes.
 * The mapping of which column index holds which attribute (e.g., 0=Name, 1=StartBit)
 * is managed by the logic in DbcModel::data().
 */
class DbcItem
{
   public:
    /**
     * @brief Constructs a new item.
     * @param data The column data for this item (Name, Value, Unit, etc.).
     * @param type The semantic type (ECU, Message, Signal).
     * @param parent The parent item (nullptr for root).
     */
    explicit DbcItem(const QList<QVariant>& data, Core::DbcItemType type,
                     DbcItem* parent = nullptr);

    /**
     * @brief Destructor. Automatically destroys all children (via unique_ptr).
     */
    ~DbcItem() = default;

    /**
     * @brief Setter for the parent item.
     * @param parent The new parent.
     */
    auto setParent(DbcItem* parent) -> void;

    /**
     * Getter for the items children items.
     * @return The children of the item.
     */
    [[nodiscard]] auto getChildren() const -> const std::vector<std::unique_ptr<DbcItem>>&;

    // --- Tree Structure Management ---
    /**
     * @brief Adds a child node to this item.
     * The item takes ownership of the child.
     */
    void appendChild(std::unique_ptr<DbcItem> child);

    /**
     * @brief Adds a child node to this item as the first child item.
     * @param child Pointer to new child
     */
    void prependChild(std::unique_ptr<DbcItem> child);

    /**
     * @brief Returns the child at the specific row index.
     * @return Pointer to the child or nullptr if index is out of bounds.
     */
    [[nodiscard]] auto child(int row) const -> DbcItem*;

    /**
     * @brief Returns the number of children (rows).
     */
    [[nodiscard]] auto childCount() const -> int;

    /**
     * @brief Returns the parent item.
     */
    [[nodiscard]] auto parent() const -> DbcItem*;

    /**
     * @brief Returns the row index of this item relative to its parent.
     * Returns 0 if this is the root item.
     */
    [[nodiscard]] auto row() const -> int;

    // --- Data Access ---

    /**
     * @brief Returns the number of data columns stored in this item.
     */
    [[nodiscard]] auto columnCount() const -> int;

    /**
     * @brief Returns the data stored at the specific column index.
     * Used by the Model to fulfill data() requests.
     */
    [[nodiscard]] auto data(int column) const -> QVariant;

    /**
     * @brief Returns the semantic type of the item.
     * Used by Proxies to filter items (e.g., show only Messages).
     */
    [[nodiscard]] auto type() const -> Core::DbcItemType;

   private:
    Core::DbcItemType m_type;
    DbcItem* m_parent;  // Raw pointer (non-owning) to prevent circular references

    // Ownership of children is managed here
    std::vector<std::unique_ptr<DbcItem>> m_children;

    // Stores values for: Name, ID/StartBit, DLC/Length, etc.
    QList<QVariant> m_data;
};

}  // namespace DbcFile