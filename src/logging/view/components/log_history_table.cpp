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
                                   "   background-color: transparent;"
                                   "   font-size: %9px;"
                                   "   font-weight: %1;"
                                   "   padding: %8px;"
                                   "   outline: none;"
                                   "}"
                                   "QTreeView::item {"
                                   "   border: none;"
                                   "   padding: %2px;"
                                   "   margin-bottom: %3px;"
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
                                   "   background: transparent;"
                                   "   border: none;"
                                   "}"
                                   "QHeaderView::section {"
                                   "   background: transparent;"
                                   "   border: none;"
                                   "   border-bottom: %4px solid %5;"
                                   "   padding: %3px %6px;"
                                   "   font-size: %9px;"
                                   "   font-weight: %7;"
                                   "   color: %10;"
                                   "}")
                                   .arg(spacing.fontWeightNormal)
                                   .arg(spacing.spacingXs + 1)
                                   .arg(spacing.spacingMd)
                                   .arg(spacing.borderThick)
                                   .arg(colors.borderSubtle.name(QColor::HexArgb))
                                   .arg(spacing.spacingMd - 2)
                                   .arg(spacing.fontWeightMedium)
                                   .arg(spacing.spacingXl)
                                   .arg(spacing.fontSizeMd)
                                   .arg(colors.textPrimary.name());
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
