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

#include "../delegates/sidebar_delegate.hpp"

#include <QAbstractItemView>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>

#include "core/macro/theme.hpp"
#include "core/theme/theme_manager.hpp"
#include "dbc_file/constants.hpp"
namespace Core {
void SidebarDelegate::setToolTipText(const QString& toolTipText)
{
    m_toolTipText = toolTipText;
}
void SidebarDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if (option->state & QStyle::State_Selected)
    {
        option->font.setBold(true);
    }
    // Get color theme
    const auto& colors = THEME.colors();
    // Get icon from index
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (icon.isNull())
    {
        return;
    }
    // Choose icon color depending on selected or unselected
    QColor iconColor;
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        // Item disabled
        iconColor = colors.textDisabled;
        option->palette.setColor(QPalette::Text, colors.textDisabled);

    } else if (option->state & QStyle::State_Selected)
    {
        // Item selected
        iconColor = colors.textPrimary;

    } else
    {
        // Item not selected
        iconColor = colors.textSecondary;
    }

    // Paint item as pixmap with correct color
    QPixmap pixmap = icon.pixmap(option->decorationSize);
    QPainter p(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), iconColor);
    p.end();
    option->icon = QIcon(pixmap);
}

auto SidebarDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    -> QSize
{
    // Enforce a minimum row height to ensure consistent spacing
    // and sufficient click target size in the sidebar.
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(size.height(), 50));
    return size;
}
auto SidebarDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                const QStyleOptionViewItem& option, const QModelIndex& index)
    -> bool
{
    // Only show a tooltip for disabled items.
    // Enabled items are expected to be self-explanatory or interactive.
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        // Handle tooltip events explicitly to prevent default behavior
        // and ensure consistent tooltip text for disabled entries.
        if (event->type() == QEvent::ToolTip)
        {
            QToolTip::showText(event->globalPos(), m_toolTipText, view);
            return true;
        }
    }
    return false;
}
}  // namespace Core