/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
    if (type == Core::DbcItemType::Overview || type == Core::DbcItemType::OrphanHolder ||
        type == Core::DbcItemType::Signal)
        return false;

    // Apply category filter
    if (m_filterCategory == Constants::EcusPage::FilterActiveIndex)
    {
        if (type == Core::DbcItemType::Ecu && index.data(Role_ChildCount).toInt() == 0)
            return false;
    }

    // 3. Apply text search filter
    if (m_filterText.isEmpty()) return true;

    // Case A: It's an ECU
    // We strictly check if the ECU name matches
    if (type == Core::DbcItemType::Ecu)
    {
        const QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
        return name.contains(m_filterText, Qt::CaseInsensitive);
    }

    // Case B: It's a Message (Child of an ECU)
    if (type == Core::DbcItemType::Message)
    {
        // sourceParent corresponds to the ECU node
        const QString parentName = sourceModel()->data(sourceParent, Qt::DisplayRole).toString();
        return parentName.contains(m_filterText, Qt::CaseInsensitive);
    }

    return false;
}

int EcuTreeProxy::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

}  // namespace DbcFile