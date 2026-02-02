//
// Created by Adrian Rupp on 22.01.26.
//
#include "signals_page.hpp"

#include <qstandarditemmodel.h>
namespace DbcFile {
// --- SignalsPage Dummy ---
SignalsPage::SignalsPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
void SignalsPage::setModel(QAbstractItemModel* model) {}
auto SignalsPage::getFilterCombo() const -> QComboBox*
{
    return nullptr;
}
void SignalsPage::setupUi()
{
};
}  // namespace DbcFile