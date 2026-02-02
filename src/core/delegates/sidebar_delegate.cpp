//
// Created by Adrian Rupp on 28.01.26.
//
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
    QColor iconColor =
        option->state & QStyle::State_Selected ? colors.textPrimary : colors.textSecondary;

    // Paint item as pixmap with correct color
    QPixmap pixmap = icon.pixmap(option->decorationSize);
    QPainter p(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), iconColor);
    p.end();
    option->icon = QIcon(pixmap);
}

auto SidebarDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
{
    // Enforce a minimum row height to ensure consistent spacing
    // and sufficient click target size in the sidebar.
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(size.height(), 50));
    return size;
}
auto SidebarDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index) -> bool
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