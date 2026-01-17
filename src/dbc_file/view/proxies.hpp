//
// Created by Adrian Rupp on 30.12.25.
//
#pragma once

#include <QAbstractProxyModel>
#include <QList>
#include <QPersistentModelIndex>
#include <QSortFilterProxyModel>

// Required for DbcItemType enum & Roles
#include "../model/dbc_roles.hpp"
#include "core/enum/dbc_itemtype.hpp"

namespace DbcFile {

// ==============================================================================
// 1. Flat List Proxy
// ==============================================================================

/**
 * @class FlatListProxy
 * @brief Flattens a hierarchical tree structure into a linear list based on item type.
 *
 * @details
 * **USE CASE:**
 * Used for "Overview Page" lists (ECUs, Messages) and "Messages/Signals Page" tables.
 *
 * **MECHANISM:**
 * It does NOT use QSortFilterProxyModel because it structurally transforms the data
 * (Tree -> List). It performs a manual recursive scan to find all items of `m_targetType`.
 */
class FlatListProxy : public QAbstractProxyModel
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the proxy for a specific target type.
     * @param targetType The DbcItemType to collect (e.g. Message).
     * @param parent Parent object.
     */
    explicit FlatListProxy(Core::DbcItemType targetType, QObject* parent = nullptr);
    ~FlatListProxy() override = default;

    /**
     * @brief Updates the search filter. Triggers rebuildMapping().
     * @caller DbcView::on...FilterTextChanged().
     * @param text Case-insensitive search string.
     */
    void setSearchFilter(const QString& text);

    /**
     * @brief Updates the category filter. Triggers rebuildMapping().
     * @caller DbcView::on...FilterTypeChanged().
     * @param index The selected index from the UI ComboBox (0 = All).
     */
    void setFilterCategory(int index);

    // --- QAbstractProxyModel Interface Implementation ---

    /**
     * @brief Maps Source Index (Tree) -> Proxy Index (List).
     * @caller Qt Views (selection handling) and Delegates.
     */
    [[nodiscard]] auto mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex override;

    /**
     * @brief Maps Proxy Index (List) -> Source Index (Tree).
     * @caller DbcView (to get the real index for Master-Detail selection) and Delegates.
     */
    [[nodiscard]] auto mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex override;

    [[nodiscard]] auto index(int row, int column, const QModelIndex& parent = QModelIndex()) const
        -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    /**
     * @brief Sets the source model and connects signals for auto-updates.
     * @caller DbcView::setSourceModel().
     */
    void setSourceModel(QAbstractItemModel* sourceModel) override;

   public slots:
    /**
     * @brief Scans the source tree and rebuilds the internal index list.
     * @caller
     * - Internal: When filters change via setSearchFilter/setFilterCategory.
     * - Signal: When sourceModel emits modelReset or layoutChanged.
     */
    void rebuildMapping();

   private:
    /**
     * @brief Recursive scanner helper.
     * @caller Internal (rebuildMapping).
     * @details Scans the tree and appends matching indices to m_mapping if they match
     * Type, SearchText, and FilterCategory.
     */
    void scanNode(const QModelIndex& parent);

    Core::DbcItemType m_targetType;
    QString m_filterText;
    int m_filterCategory = 0;

    /** @brief The flattened list of persistent pointers to the source items. */
    QList<QPersistentModelIndex> m_mapping;
};

// ==============================================================================
// 2. Tree Filter Proxy
// ==============================================================================

/**
 * @class TreeFilterProxy
 * @brief A proxy that maintains the tree hierarchy but filters nodes by text and type.
 *
 * @details
 * **USE CASE:**
 * Used for the "ECUs Page" (Tree View).
 *
 * **LOGIC:**
 * - Hides the "Overview" metadata row.
 * - Filters ECUs/Messages based on search text (Search Bar).
 * - Filters ECUs based on category (Filter Box).
 * - Maintains parent-child relationship (Recursive Filtering).
 */
class TreeFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

   public:
    explicit TreeFilterProxy(QObject* parent = nullptr);
    ~TreeFilterProxy() override = default;

    /**
     * @brief Updates the search filter string.
     * @caller DbcView::onEcuFilterTextChanged().
     */
    void setSearchFilter(const QString& text);

    /**
     * @brief Updates the category filter (e.g., "Sending Nodes").
     * @caller DbcView::onEcuFilterTypeChanged().
     */
    void setFilterCategory(int index);

   protected:
    /**
     * @brief Decides if a row is included in the view.
     * @caller Qt Internal (QSortFilterProxyModel) whenever the filter changes.
     *
     * Logic:
     * 1. Hides `Role_ItemType == Overview`.
     * 2. Checks search text against DisplayRole.
     * 3. Checks category index against model data.
     */
    [[nodiscard]] auto filterAcceptsRow(int sourceRow,
                                        const QModelIndex& sourceParent) const -> bool override;

   private:
    QString m_filterText;
    int m_filterCategory = 0;
};

// ==============================================================================
// 3. Single Message Proxy
// ==============================================================================

/**
 * @class SingleMessageProxy
 * @brief A proxy that isolates the children of a specific parent node.
 *
 * @details
 * **USE CASE:**
 * Used for the "Message Detail View" (Bottom Pane of Messages Tab).
 * It displays ALL signals belonging to the currently selected Message.
 *
 * **LOGIC:**
 * It filters out everything except the direct children (Signals) of the
 * configured parent index.
 */
class SingleMessageProxy : public QSortFilterProxyModel
{
    Q_OBJECT

   public:
    explicit SingleMessageProxy(QObject* parent = nullptr);
    ~SingleMessageProxy() override = default;

    /**
     * @brief Sets the target message whose signals should be displayed.
     * @caller DbcView::onMessageSelected().
     * @param parentIndex The index of the Message in the source model.
     *
     * Calling this triggers an invalidation of the filter.
     */
    void setFilterParentIndex(const QModelIndex& parentIndex);

   protected:
    /**
     * @brief Decides if a row is included.
     * @caller Qt Internal (QSortFilterProxyModel).
     *
     * Logic: Returns true ONLY if `sourceParent == m_parentIndex`.
     */
    [[nodiscard]] auto filterAcceptsRow(int sourceRow,
                                        const QModelIndex& sourceParent) const -> bool override;

   private:
    QModelIndex m_parentIndex;
};

}  // namespace DbcFile