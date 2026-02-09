#include "proxies.hpp"

#include <qopenglcontext_platform.h>

namespace DbcFile {
FlatListProxy::FlatListProxy(const Core::DbcItemType targetType, QObject* parent)
    : QAbstractProxyModel(parent), m_targetType(targetType)
{
}
void FlatListProxy::setSearchFilter(const QString& text)
{
    if (m_filterText == text) return;
    m_filterText = text;
    rebuildMapping();
}
void FlatListProxy::setFilterCategory(const int index)
{
    if (m_filterCategory == index) return;
    m_filterCategory = index;
    rebuildMapping();
}
void FlatListProxy::rebuildMapping()
{
    if (!sourceModel()) return;

    beginResetModel();
    m_mapping.clear();

    scanNode(QModelIndex());

    endResetModel();
}

auto FlatListProxy::mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex
{
    if (!sourceIndex.isValid()) return {};

    // Linear search
    for (int i = 1; i < m_mapping.size(); i++)
    {
        if (m_mapping.at(i) == sourceIndex)
        {
            return index(i, sourceIndex.column());
        }
    }
    return {};  // item not found
}
auto FlatListProxy::mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex
{
    if (!proxyIndex.isValid() || proxyIndex.row() >= m_mapping.size()) return {};

    // Get source index
    const QPersistentModelIndex sourceIdx = m_mapping.at(proxyIndex.row());

    // Return right column
    return sourceIdx.sibling(sourceIdx.row(), proxyIndex.column());
}
auto FlatListProxy::index(const int row, const int column,
                          const QModelIndex& parent) const -> QModelIndex
{
    // No parents in flat list
    if (parent.isValid() || row >= m_mapping.size()) return {};

    // No data stored in index, data will later be retrieved from source index via mapToSource
    return createIndex(row, column);
}
auto FlatListProxy::parent(const QModelIndex& child) const -> QModelIndex
{
    return {};  // No parents in flat list
}
auto FlatListProxy::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) return 0;  // if parent valid -> no children in flat list
    return m_mapping.size();         // if parent invalid -> return amount of items
}
auto FlatListProxy::columnCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) return 0;
    return sourceModel() ? sourceModel()->columnCount() : 0;  // return column count of source model
}
void FlatListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
    // Disconnect old model
    if (this->sourceModel())
    {
        disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    // Set new model
    QAbstractProxyModel::setSourceModel(sourceModel);

    // Connect new model to proxy
    if (sourceModel)
    {
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &FlatListProxy::rebuildMapping);
    }

    // Rebuild Mapping with new Model
    rebuildMapping();
}
void FlatListProxy::scanNode(const QModelIndex& parent)
{
    // Base case: rows = 0
    const int rows = sourceModel()->rowCount(parent);
    for (int i = 0; i < rows; i++)
    {
        // Get current index and type
        QModelIndex currentInd = sourceModel()->index(i, 0, parent);
        auto type = static_cast<Core::DbcItemType>(
            sourceModel()->data(currentInd, Role_ItemType).toInt());

        // Item at current index has correct type -> add mapping
        if (type == m_targetType)
        {
            bool accept = true;

            // Text filter
            if (!m_filterText.isEmpty())
            {
                if (QString itemName = sourceModel()->data(currentInd, Qt::DisplayRole).toString();
                    !itemName.contains(m_filterText, Qt::CaseInsensitive))
                {
                    accept = false;
                }
            }

            // Category filter
            if (accept == true)
            {
                m_mapping.append(QPersistentModelIndex(currentInd));
            }
        }

        bool recurse = true;

        // ECUs/Messages cant have ECUs/Messages for children
        if (m_targetType == Core::DbcItemType::Ecu && type == Core::DbcItemType::Ecu)
        {
            recurse = false;
        }
        if (m_targetType == Core::DbcItemType::Message && type == Core::DbcItemType::Message)
        {
            recurse = false;
        }

        // skip overview item
        if (type == Core::DbcItemType::Overview)
        {
            recurse = false;
        }

        if (recurse && sourceModel()->hasChildren(currentInd))
        {
            scanNode(currentInd);
        }
    }
}
}