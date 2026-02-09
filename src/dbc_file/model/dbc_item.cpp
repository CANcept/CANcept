#include "dbc_item.hpp"

namespace DbcFile {
DbcFile::DbcItem::DbcItem(const QList<QVariant>& data, Core::DbcItemType type, DbcItem* parent)
    : m_type(type), m_parent(parent), m_data(data)
{
}

auto DbcItem::setParent(DbcItem* parent) -> void
{
    m_parent = parent;
}
void DbcItem::setData(int column, const QVariant& value) {
    if (column < 0) return;
    if (column >= m_data.size())
        m_data.resize(column + 1);
    m_data[column] = value;
}

const std::vector<std::unique_ptr<DbcItem>>& DbcItem::getChildren() const
{
    return m_children;
}

void DbcFile::DbcItem::appendChild(std::unique_ptr<DbcItem> child)
{
    child->m_parent = this;
    m_children.push_back(std::move(child));
}
void DbcItem::prependChild(std::unique_ptr<DbcItem> child)
{
    child->m_parent = this;
    m_children.insert(m_children.begin(), std::move(child));
}
auto DbcFile::DbcItem::child(int row) const -> DbcItem*
{
    if (m_children.empty() || row >= static_cast<int>(m_children.size()) || row < 0)
    {
        return nullptr;
    }
    return m_children[row].get();
}
auto DbcFile::DbcItem::childCount() const -> int
{
    return static_cast<int>(m_children.size());
}
auto DbcFile::DbcItem::parent() const -> DbcItem*
{
    return m_parent;
}
auto DbcFile::DbcItem::row() const -> int
{
    if (m_parent)
    {
        const auto& siblings = m_parent->m_children;
        for (int i = 0; i < static_cast<int>(siblings.size()); i++)
        {
            if (siblings[i].get() == this)
            {
                return i;
            }
        }
    }
    return 0;  // if item has no parent it is row 0
}
auto DbcFile::DbcItem::columnCount() const -> int
{
    return static_cast<int>(m_data.size());
}
auto DbcFile::DbcItem::data(int column) const -> QVariant
{
    if (column < 0 || column >= m_data.size())
    {
        return QVariant();  // return empty QVariant at invalid column
    }
    return m_data[column];
}

auto DbcFile::DbcItem::type() const -> Core::DbcItemType
{
    return m_type;
}
}  // namespace DbcFile