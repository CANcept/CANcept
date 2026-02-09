#include "searchable_filter_widgets.hpp"

namespace DbcFile {

// --- SearchableFilterTable ---
SearchableFilterTable::SearchableFilterTable(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
QTableView* SearchableFilterTable::tableView() const
{
    return m_tableView;
}
QLineEdit* SearchableFilterTable::searchBar() const
{
    return m_searchBar;
}
QComboBox* SearchableFilterTable::filterComboBox() const
{
    return m_filterCombo;
}
void SearchableFilterTable::setupUi()
{
    m_tableView = new QTableView(this);
    m_searchBar = new QLineEdit(this);
    m_filterCombo = new QComboBox(this);
}

// --- SearchableFilterTree ---
SearchableFilterTree::SearchableFilterTree(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
QTreeView* SearchableFilterTree::treeView() const
{
    return m_treeView;
}
QLineEdit* SearchableFilterTree::searchBar() const
{
    return m_searchBar;
}
QComboBox* SearchableFilterTree::filterComboBox() const
{
    return m_filterCombo;
}
void SearchableFilterTree::setupUi()
{
    m_treeView = new QTreeView(this);
    m_searchBar = new QLineEdit(this);
    m_filterCombo = new QComboBox(this);
}

}  // namespace DbcFile