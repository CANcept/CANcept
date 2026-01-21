//
// Created by Adrian Rupp on 22.01.26.
//
#include "overview_page.hpp"
namespace DbcFile {
// --- OverviewPage Dummy ---
OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
void OverviewPage::setupUi()
{ /* Leer lassen */
}
auto OverviewPage::createStatCard(const QString& title, QLabel*& valueLabelPtr,
                                  const QString& iconName) -> QWidget*
{
    return new QWidget(this);
}

}  // namespace DbcFile
