//
// Created by Adrian Rupp on 22.01.26.
//
#include "ecus_page.hpp"
namespace DbcFile {
// --- EcusPage Dummy ---
EcusPage::EcusPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
void EcusPage::setModel(QAbstractItemModel* model) {}
auto EcusPage::getFilterCombo() const -> QComboBox*
{
    return nullptr;
}
void EcusPage::setupUi()
{
    m_treeWidget = new SearchableFilterTree(this);

}
}  // namespace DbcFile