#include "log_history_table.hpp"

#include <QHeaderView>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Logging {

LogHistoryTable::LogHistoryTable(QWidget* parent) : QTreeView(parent)
{
    setupUi();
    applyStyle();
}

void LogHistoryTable::setupUi()
{
    setRootIsDecorated(false);
    setAlternatingRowColors(false);
    setSelectionBehavior(SelectRows);
    setSelectionMode(SingleSelection);
    setEditTriggers(NoEditTriggers);
    setFrameShape(NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Configure header
    QHeaderView* header = this->header();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::Stretch);
}

void LogHistoryTable::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    const QString tableStyle = QString(
                                   "QTreeView {"
                                   "   border: none;"
                                   "   background-color: %1;"
                                   "   font-size: %11px;"
                                   "   font-weight: %2;"
                                   "   padding: %10px;"
                                   "   outline: none;"
                                   "}"
                                   "QTreeView::item {"
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
                                   "   font-size: %11px;"
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
                                   .arg(colors.textPrimary.name())
                                   .arg(spacing.spacingXl)
                                   .arg(spacing.fontSizeMd);
    setStyleSheet(tableStyle);
}

bool LogHistoryTable::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QTreeView::event(event);
}

}  // namespace Logging
