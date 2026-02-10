#include "single_message_proxy.hpp"
namespace DbcFile {
SingleMessageProxy::SingleMessageProxy(QObject* parent) : QSortFilterProxyModel(parent) {}
void SingleMessageProxy::setFilterParentIndex(const QModelIndex& parentIndex) {}
bool SingleMessageProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    return true;
}
}