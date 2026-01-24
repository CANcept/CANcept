#include "app_root/model/app_root_model.hpp"

namespace AppRoot {

AppRootModel::AppRootModel(QObject* parent) : QAbstractListModel(parent) {}

auto AppRootModel::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_tabs.size());
}

auto AppRootModel::data(const QModelIndex& index, const int role) const -> QVariant
{
    if (!index.isValid() || index.row() >= m_tabs.size())
    {
        return {};
    }

    auto* component = m_tabs.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return component->getTitle();
        case Qt::DecorationRole:
            return component->getIcon();
        case IdRole:
            return component->getId();
        case ComponentRole:
            return QVariant::fromValue(component);
        default:
            return {};
    }
}
void AppRootModel::addTab(Core::ITabComponent* component)
{
    if (!component)
    {
        return;
    }
    const int row = static_cast<int>(m_tabs.size());
    beginInsertRows(QModelIndex(), row, row);
    m_tabs.append(component);
    endInsertRows();

    // connect the tab components update method to the data changed event
    connect(component, &Core::ITabComponent::updated, this, [this, component]() {
        if (const int current_row = m_tabs.indexOf(component); current_row != -1)
        {
            const QModelIndex index = this->index(current_row);
            emit dataChanged(index, index);
        }
    });
}

void AppRootModel::removeTab(const QString& id)
{
    for (int i = 0; i < m_tabs.size(); ++i)
    {
        if (m_tabs.at(i)->getId() == id)
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_tabs.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}

void AppRootModel::replaceTab(Core::ITabComponent* old_tab, Core::ITabComponent* new_tab)
{
    if (const int row = m_tabs.indexOf(old_tab); row != -1)
    {
        m_tabs[row] = new_tab;
        const QModelIndex index = this->index(row);
        emit dataChanged(index, index);
    }
}

auto AppRootModel::componentAt(const int row) const -> Core::ITabComponent*
{
    if (row < 0 || row >= m_tabs.size())
    {
        return nullptr;
    }
    return m_tabs.at(row);
}

}  // namespace AppRoot