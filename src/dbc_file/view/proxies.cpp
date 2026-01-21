//
// Created by Adrian Rupp on 20.01.26.
//
#include "proxies.hpp"

namespace DbcFile {  // Oder namespace DbcFile, je nachdem was im Header steht!

// --- FlatListProxy (Dummy) ---

FlatListProxy::FlatListProxy(Core::DbcItemType targetType, QObject* parent)
    : QAbstractProxyModel(parent)
{
}

void FlatListProxy::setSearchFilter(const QString& text) {}
void FlatListProxy::setFilterCategory(int index) {}
void FlatListProxy::rebuildMapping() {}

// Wichtig: Die puren virtuellen Methoden müssen existieren!
QModelIndex FlatListProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
    return {};
}
QModelIndex FlatListProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    return {};
}
QModelIndex FlatListProxy::index(int row, int column, const QModelIndex& parent) const
{
    return {};
}
QModelIndex FlatListProxy::parent(const QModelIndex& child) const
{
    return {};
}
int FlatListProxy::rowCount(const QModelIndex& parent) const
{
    return 0;
}
int FlatListProxy::columnCount(const QModelIndex& parent) const
{
    return 0;
}
void FlatListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
    QAbstractProxyModel::setSourceModel(sourceModel);
}
void FlatListProxy::scanNode(const QModelIndex& parent) {}

// --- TreeFilterProxy (Dummy) ---

TreeFilterProxy::TreeFilterProxy(QObject* parent) : QSortFilterProxyModel(parent) {}
void TreeFilterProxy::setSearchFilter(const QString& text) {}
void TreeFilterProxy::setFilterCategory(int index) {}
bool TreeFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    return true;
}

// --- SingleMessageProxy (Dummy) ---

SingleMessageProxy::SingleMessageProxy(QObject* parent) : QSortFilterProxyModel(parent) {}
void SingleMessageProxy::setFilterParentIndex(const QModelIndex& parentIndex) {}
bool SingleMessageProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    return true;
}

}  // namespace DbcFile
