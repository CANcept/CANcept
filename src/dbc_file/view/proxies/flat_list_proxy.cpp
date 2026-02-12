#include "flat_list_proxy.hpp"

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

// -----------------------------------------------------------------------------
// Construction / filters
// -----------------------------------------------------------------------------

FlatListProxy::FlatListProxy(Core::DbcItemType targetType, QObject* parent)
    : QAbstractProxyModel(parent), m_targetType(targetType)
{
}

void FlatListProxy::setSearchFilter(const QString& text)
{
    if (m_filterText == text) return;
    m_filterText = text;
    rebuildMapping();
}

void FlatListProxy::setSignalFilterUnit(const QString& unit)
{
    if (m_filterSignalUnit == unit) return;
    m_filterSignalUnit = unit;
    rebuildMapping();
}

void FlatListProxy::setFilterMessageSender(const QString& sender)
{
    if (m_filterMessageSender == sender) return;
    m_filterMessageSender = sender;
    rebuildMapping();
}

// -----------------------------------------------------------------------------
// QAbstractProxyModel interface
// -----------------------------------------------------------------------------

auto FlatListProxy::columnCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) return 0;

    switch (m_targetType)
    {
        case Core::DbcItemType::Signal:
            return Constants::Columns::SignalColumnCount;
        case Core::DbcItemType::Message:
            return Constants::Columns::MsgColumnCount;
        default:
            // Fallback (e.g. ECU): mirror the source model if available.
            return sourceModel() ? sourceModel()->columnCount() : 0;
    }
}

auto FlatListProxy::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QAbstractProxyModel::headerData(section, orientation, role);
    }

    if (m_targetType == Core::DbcItemType::Message)
    {
        switch (section)
        {
            case Constants::Columns::MsgId:
                return Constants::Headers::MsgId;
            case Constants::Columns::MsgName:
                return Constants::Headers::MsgName;
            case Constants::Columns::MsgSender:
                return Constants::Headers::MsgSender;
            case Constants::Columns::MsgDlc:
                return Constants::Headers::MsgDlc;
            case Constants::Columns::MsgSigCount:
                return Constants::Headers::MsgSigCount;
            default:
                return {};
        }
    }

    if (m_targetType == Core::DbcItemType::Signal)
    {
        switch (section)
        {
            case Constants::Columns::SigName:
                return Constants::Headers::SigName;
            case Constants::Columns::SigMessage:
                return Constants::Headers::SigMessage;
            case Constants::Columns::SigStartBit:
                return Constants::Headers::SigStartBit;
            case Constants::Columns::SigUnit:
                return Constants::Headers::SigUnit;
            case Constants::Columns::SigLength:
                return Constants::Headers::SigLength;
            case Constants::Columns::SigMin:
                return Constants::Headers::SigRange;  // (your current mapping)
            case Constants::Columns::SigFactor:
                return Constants::Headers::SigFactor;
            case Constants::Columns::SigOffset:
                return Constants::Headers::SigOffset;
            case Constants::Columns::SigByteOrder:
                return Constants::Headers::SigByteOrder;
            case Constants::Columns::SigValueType:
                return Constants::Headers::SigType;
            default:
                return {};
        }
    }

    return {};
}

void FlatListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
    if (this->sourceModel())
    {
        disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel)
    {
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &FlatListProxy::rebuildMapping);
    }

    rebuildMapping();
}

void FlatListProxy::rebuildMapping()
{
    if (!sourceModel()) return;

    beginResetModel();

    m_mapping.clear();
    m_indexLookup.clear();

    scanNode(QModelIndex{});

    // Build lookup table for O(1) mapFromSource.
    for (int row = 0; row < m_mapping.size(); ++row)
    {
        m_indexLookup[m_mapping[row]] = row;
    }

    endResetModel();
}

auto FlatListProxy::mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex
{
    if (!sourceIndex.isValid()) return {};

    if (auto it = m_indexLookup.find(sourceIndex); it != m_indexLookup.end())
    {
        return index(it.value(), sourceIndex.column(), QModelIndex{});
    }
    return {};
}

auto FlatListProxy::mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex
{
    if (!proxyIndex.isValid()) return {};
    if (proxyIndex.row() < 0 || proxyIndex.row() >= m_mapping.size()) return {};
    const auto& src = m_mapping[proxyIndex.row()];
    return src.sibling(src.row(), proxyIndex.column());
}

auto FlatListProxy::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    if (parent.isValid()) return {};
    if (row < 0 || row >= m_mapping.size()) return {};
    if (column < 0 || column >= columnCount(QModelIndex{})) return {};

    return createIndex(row, column);
}

auto FlatListProxy::parent(const QModelIndex&) const -> QModelIndex
{
    return {};  // Flat list: no parents.
}

auto FlatListProxy::rowCount(const QModelIndex& parent) const -> int
{
    return parent.isValid() ? 0 : m_mapping.size();
}

// -----------------------------------------------------------------------------
// Recursive scanning
// -----------------------------------------------------------------------------

void FlatListProxy::scanNode(const QModelIndex& parent)
{
    if (!sourceModel()) return;

    const int rows = sourceModel()->rowCount(parent);
    for (int i = 0; i < rows; ++i)
    {
        const QModelIndex currentIndex = sourceModel()->index(i, 0, parent);
        const auto type = static_cast<Core::DbcItemType>(
            sourceModel()->data(currentIndex, Role_ItemType).toInt());

        if (type == m_targetType && passesFilters(currentIndex))
        {
            m_mapping.append(QPersistentModelIndex(currentIndex));
        }

        // Stop recursion once we reached the target type level (or overview).
        const bool stopHere =
            type == Core::DbcItemType::Overview ||
            (m_targetType == Core::DbcItemType::Ecu && type == Core::DbcItemType::Ecu) ||
            (m_targetType == Core::DbcItemType::Message && type == Core::DbcItemType::Message);

        if (!stopHere && sourceModel()->hasChildren(currentIndex))
        {
            scanNode(currentIndex);
        }
    }
}

// -----------------------------------------------------------------------------
// Filter helpers
// -----------------------------------------------------------------------------

auto FlatListProxy::passesFilters(const QModelIndex& idx) const -> bool
{
    return passesUnitFilter(idx) && passesTextFilter(idx) && passesSenderFilter(idx);
}

auto FlatListProxy::passesUnitFilter(const QModelIndex& idx) const -> bool
{
    if (m_targetType != Core::DbcItemType::Signal || m_filterSignalUnit.isEmpty())
    {
        return true;
    }

    const QString unit = sourceModel()->data(idx, Role_Unit).toString();
    return unit == m_filterSignalUnit;
}

auto FlatListProxy::passesSenderFilter(const QModelIndex& idx) const -> bool
{
    if (m_targetType != Core::DbcItemType::Message || m_filterMessageSender.isEmpty())
    {
        return true;
    }

    const QString sender = sourceModel()->data(idx, Role_Sender).toString();
    return sender == m_filterMessageSender;
}

auto FlatListProxy::passesTextFilter(const QModelIndex& idx) const -> bool
{
    if (m_filterText.isEmpty()) return true;

    const QString name = sourceModel()->data(idx, Qt::DisplayRole).toString();
    return name.contains(m_filterText, Qt::CaseInsensitive);
}

}  // namespace DbcFile
