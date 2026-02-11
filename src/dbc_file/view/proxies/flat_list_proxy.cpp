#include "flat_list_proxy.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

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

void FlatListProxy::setSignalFilterUnit(const QString& unit)
{
    if (m_filterSignalUnit == unit) return;
    m_filterSignalUnit = unit;
    rebuildMapping();
}

// -----------------------------------------------------------------------------
// QAbstractProxyModel interface
// -----------------------------------------------------------------------------

auto FlatListProxy::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractProxyModel::headerData(section, orientation, role);

    static const QMap<int, QString> msgHeaders{
        {Constants::Columns::MsgId, Constants::Headers::MsgId},
        {Constants::Columns::MsgName, Constants::Headers::MsgName},
        {Constants::Columns::MsgSender, Constants::Headers::MsgSender},
        {Constants::Columns::MsgDlc, Constants::Headers::MsgDlc},
    };

    static const QMap<int, QString> sigHeaders{
        {Constants::Columns::SigName, Constants::Headers::SigName},
        {Constants::Columns::SigMessage, Constants::Headers::SigMessage},
        {Constants::Columns::SigStartBit, Constants::Headers::SigStartBit},
        {Constants::Columns::SigUnit, Constants::Headers::SigUnit},
        {Constants::Columns::SigLength, Constants::Headers::SigLength},
        {Constants::Columns::SigMin, Constants::Headers::SigRange},
        {Constants::Columns::SigFactor, Constants::Headers::SigFactor},
        {Constants::Columns::SigOffset, Constants::Headers::SigOffset},
        {Constants::Columns::SigByteOrder, Constants::Headers::SigByteOrder},
        {Constants::Columns::SigValueType, Constants::Headers::SigType},
    };

    if (m_targetType == Core::DbcItemType::Message)
        return msgHeaders.value(section, "");
    if (m_targetType == Core::DbcItemType::Signal)
        return sigHeaders.value(section, "");

    return {};
}

void FlatListProxy::rebuildMapping()
{
    if (!sourceModel()) return;

    beginResetModel();
    m_mapping.clear();
    m_indexLookup.clear();

    scanNode(QModelIndex());

    // build lookup table for fast mapFromSource
    for (int row = 0; row < m_mapping.size(); ++row)
    {
        m_indexLookup[m_mapping[row]] = row;
    }

    endResetModel();
}

auto FlatListProxy::mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex
{
    if (!sourceIndex.isValid()) return {};

    // Use lookup table for O(1)
    if (auto it = m_indexLookup.find(sourceIndex); it != m_indexLookup.end())
        return index(it.value(), sourceIndex.column(),sourceIndex);

    return {}; // not found
}

auto FlatListProxy::mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex
{
    if (!proxyIndex.isValid() || proxyIndex.row() >= m_mapping.size()) return {};
    return m_mapping[proxyIndex.row()].sibling(m_mapping[proxyIndex.row()].row(), proxyIndex.column());
}

auto FlatListProxy::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    if (parent.isValid() || row >= m_mapping.size()) return {};
    return createIndex(row, column);
}

auto FlatListProxy::parent(const QModelIndex&) const -> QModelIndex
{
    return {}; // flat list: no parents
}

auto FlatListProxy::rowCount(const QModelIndex& parent) const -> int
{
    return parent.isValid() ? 0 : m_mapping.size();
}

auto FlatListProxy::columnCount(const QModelIndex& parent) const -> int
{
    return parent.isValid() ? 0 : (sourceModel() ? sourceModel()->columnCount() : 0);
}

void FlatListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
    if (this->sourceModel())
        disconnect(this->sourceModel(), nullptr, this, nullptr);

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel)
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &FlatListProxy::rebuildMapping);

    rebuildMapping();
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
        QModelIndex currentIndex = sourceModel()->index(i, 0, parent);
        auto type = static_cast<Core::DbcItemType>(
            sourceModel()->data(currentIndex, Role_ItemType).toInt());

        if (type == m_targetType && passesFilters(currentIndex))
            m_mapping.append(QPersistentModelIndex(currentIndex));

        // Determine if children should be scanned
        bool recurse = true;
        if ((m_targetType == Core::DbcItemType::Ecu && type == Core::DbcItemType::Ecu) ||
            (m_targetType == Core::DbcItemType::Message && type == Core::DbcItemType::Message) ||
            type == Core::DbcItemType::Overview)
        {
            recurse = false;
        }

        if (recurse && sourceModel()->hasChildren(currentIndex))
            scanNode(currentIndex);
    }
}

// -----------------------------------------------------------------------------
// Filter helpers
// -----------------------------------------------------------------------------

auto FlatListProxy::passesFilters(const QModelIndex& idx) const -> bool
{
    return passesUnitFilter(idx) && passesTextFilter(idx);
}

auto FlatListProxy::passesUnitFilter(const QModelIndex& idx) const -> bool
{
    if (m_targetType != Core::DbcItemType::Signal || m_filterSignalUnit.isEmpty())
        return true;

    QString unit = sourceModel()->data(idx, Role_Unit).toString();
    return unit == m_filterSignalUnit;
}

auto FlatListProxy::passesTextFilter(const QModelIndex& idx) const -> bool
{
    if (m_filterText.isEmpty()) return true;

    QString name = sourceModel()->data(idx, Qt::DisplayRole).toString();
    return name.contains(m_filterText, Qt::CaseInsensitive);
}

} // namespace DbcFile