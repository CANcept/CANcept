#include "ecu_tree_proxy.hpp"

#include "core/enum/dbc_itemtype.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace Core {
enum class DbcItemType;
}

namespace DbcFile {

EcuTreeProxy::EcuTreeProxy(QObject* parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void EcuTreeProxy::setSearchText(const QString& text)
{
    m_filterText = text;
    invalidateFilter();
}

void EcuTreeProxy::setFilterCategory(int index)
{
    m_filterCategory = index;
    invalidateFilter();
}

auto EcuTreeProxy::hasChildren(const QModelIndex& parent) const -> bool
{
    if (parent.isValid())
    {
        auto type = static_cast<Core::DbcItemType>(parent.data(Role_ItemType).toInt());
        if (type == Core::DbcItemType::Message)
        {
            // Messages are leaf nodes
            return false;
        }
    }
    return QSortFilterProxyModel::hasChildren(parent);
}

auto EcuTreeProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const -> bool
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid()) return false;

    auto type = static_cast<Core::DbcItemType>(sourceModel()->data(index, Role_ItemType).toInt());

    // Skip overview/orphan holder nodes
    if (type == Core::DbcItemType::Overview || type == Core::DbcItemType::OrphanHolder)
        return false;

    // Apply category filter
    if (m_filterCategory == Constants::EcusPage::FilterActiveIndex)
    {
        if (type == Core::DbcItemType::Ecu && index.data(Role_ChildCount).toInt() == 0)
            return false;
    }

    // Apply text search filter (contains, case insensitive)
    const QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
    if (name.contains(m_filterText, Qt::CaseInsensitive)) return true;

    return false;
}

int EcuTreeProxy::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

}  // namespace DbcFile