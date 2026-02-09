#include "history_table.hpp"

#include <QHeaderView>

#include "core/macro/theme.hpp"

namespace Logging {

HistoryTable::HistoryTable(QWidget* parent) : QTreeView(parent)
{
    setupUi();
    applyStyle();
}

void HistoryTable::setupUi()
{
    setRootIsDecorated(false);
    setAlternatingRowColors(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Configure header
    QHeaderView* header = this->header();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::Stretch);
}

void HistoryTable::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString tableStyle = QString(
                                   "QTreeView {"
                                   "   border: none;"
                                   "   background-color: %1;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 20px;"
                                   "   font-weight: %2;"
                                   "   padding: 32px;"
                                   "   outline: none;"
                                   "}"
                                   "QTreeView::item {"
                                   "   height: 61px;"
                                   "   border: none;"
                                   "   padding: %3px;"
                                   "   margin-bottom: %4px;"
                                   "   background-color: transparent;"
                                   "}"
                                   "QTreeView::item:hover {"
                                   "   background-color: transparent;"
                                   "   border: none;"
                                   "}"
                                   "QTreeView::item:selected {"
                                   "   background-color: transparent;"
                                   "   border: none;"
                                   "}"
                                   "QTreeView::branch {"
                                   "   border: none;"
                                   "}"
                                   "QHeaderView {"
                                   "   background-color: %1;"
                                   "   border: none;"
                                   "}"
                                   "QHeaderView::section {"
                                   "   background-color: %1;"
                                   "   border: none;"
                                   "   border-bottom: %5px solid %6;"
                                   "   padding: %4px %7px;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 24px;"
                                   "   font-weight: %8;"
                                   "   color: %9;"
                                   "}")
                                   .arg(colors.surfaceMain.name())
                                   .arg(spacing.fontWeightNormal)
                                   .arg(spacing.spacingXs + 1)
                                   .arg(spacing.spacingMd)
                                   .arg(spacing.borderThin)
                                   .arg(colors.borderSubtle.name())
                                   .arg(spacing.spacingMd - 2)
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textPrimary.name());
    setStyleSheet(tableStyle);
}

}  // namespace Logging
