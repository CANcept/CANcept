#pragma once
#include <QAbstractProxyModel>
namespace Core {
enum class DbcItemType;
}
//
// Created by Adrian Rupp on 09.02.26.
//
namespace DbcFile {
/**
 * @class FlatListProxy
 * @brief Flattens a hierarchical tree structure into a linear list based on item type.
 *
 * @details
 * USE CASE:
 * Used for "Overview Page" lists (ECUs, Messages) and "Messages/Signals Page" tables.
 *
 * MECHANISM:
 * It does NOT use QSortFilterProxyModel because it structurally transforms the data
 * (Tree -> List). It performs a manual recursive scan to find all items of `m_targetType`.
 */
class FlatListProxy final : public QAbstractProxyModel
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
     * @brief Sets the text filter.
     *
     * Filters items by their DisplayRole (case-insensitive).
     * Triggers a full rebuild of the internal mapping.
     *
     * @param text Filter text
     */
    void setSearchFilter(const QString& text);

    /**
     * @brief Sets the category filter.
     *
     * Currently not implemented, but still triggers a mapping rebuild.
     *
     * @param index Category index
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
     * @brief Rebuilds the internal mapping list.
     *
     * Recursively traverses the source model and collects all matching items
     * according to the target type and filter settings.
     */
    void rebuildMapping();

   private:
    /**
     * @brief Recursive scanner helper.
     *
     * @details Scans the tree and appends matching indices to m_mapping if they match
     * Type, SearchText, and FilterCategory.
     */
    void scanNode(const QModelIndex& parent);

    Core::DbcItemType m_targetType;
    QString m_filterText;
    int m_filterCategory = -1;

    /** @brief The flattened list of persistent pointers to the source items. */
    QList<QPersistentModelIndex> m_mapping;
};
}  // namespace DbcFile
