#pragma once

#include <QAbstractProxyModel>
#include <QPersistentModelIndex>
#include <QHash>
#include <QStringList>

#include "core/enum/dbc_itemtype.hpp"

namespace DbcFile {

/**
 * @brief A proxy model that flattens hierarchical DBC models into a flat list
 *        and provides filtering capabilities for messages or signals.
 *
 * This proxy supports:
 * - Flattening hierarchical source models (ECUs, Messages, Signals) into a single-level list
 * - Text-based filtering
 * - Signal unit filtering (only for signals)
 * - Fast index mapping between source and proxy via a lookup hash
 *
 * It is intended to simplify displaying DBC data in QTableView/QTreeView
 * while maintaining filtering and selection.
 */
class FlatListProxy : public QAbstractProxyModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a FlatListProxy for a given target type (Message/Signal/ECU)
     * @param targetType The type of DBC item to expose in this flat proxy
     * @param parent Optional parent QObject
     */
    explicit FlatListProxy(const Core::DbcItemType targetType, QObject* parent = nullptr);

    /** Set the search text for filtering items */
    void setSearchFilter(const QString& text);

    /** Set the filter for signal units (only affects Signal items) */
    void setSignalFilterUnit(const QString& unit);

    /** Rebuilds the mapping from source model to proxy and updates the lookup hash */
    void rebuildMapping();

    // QAbstractProxyModel overrides
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override;
    [[nodiscard]] auto mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex override;
    [[nodiscard]] auto mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex override;
    [[nodiscard]] auto index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    void setSourceModel(QAbstractItemModel* sourceModel) override;

private:
    /** Recursively scans the source model and rebuilds the mapping of items */
    void scanNode(const QModelIndex& parent);

    /** Checks if an index passes all filters (text & unit) */
    [[nodiscard]] auto passesFilters(const QModelIndex& idx) const -> bool;

    /** Checks if the index passes the signal unit filter */
    [[nodiscard]] auto passesUnitFilter(const QModelIndex& idx) const -> bool;

    /** Checks if the index passes the text filter */
    [[nodiscard]] auto passesTextFilter(const QModelIndex& idx) const -> bool;


private:
    Core::DbcItemType m_targetType;                        ///< Target DBC item type for this proxy
    QString m_filterText;                                  ///< Current search/filter text
    QString m_filterSignalUnit;                             ///< Current signal unit filter
    QList<QPersistentModelIndex> m_mapping;               ///< Flattened mapping of source indices
    QHash<QPersistentModelIndex, int> m_indexLookup;      ///< Lookup hash for fast mapFromSource
};

} // namespace DbcFile