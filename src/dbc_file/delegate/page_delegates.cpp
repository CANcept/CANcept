//
// Created by Adrian Rupp on 21.01.26.
//
#include "page_delegates.hpp"

#include <QHelpEvent>
#include <QToolTip>

#include "core/theme/theme_manager.hpp"
#include "dbc_file/constants.hpp"
namespace DbcFile {

void SidebarDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if (option->state & QStyle::State_Selected)
    {
        option->font.setBold(true);
    }
    // Get color theme
    const auto& colors = Core::ThemeManager::getInstance().colors();

    // Get icon from index
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (icon.isNull()){return;}
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
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(std::max(s.height(), 50));
    return s;
}
auto SidebarDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                const QStyleOptionViewItem& option, const QModelIndex& index) -> bool
{
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        if (event->type() == QEvent::ToolTip)
        {
            QToolTip::showText(event->globalPos(), Constants::Sidebar::HoverText, view);
            return true;
        }
    }
    return false;
}
OverviewListsDelegate::OverviewListsDelegate(QObject* parent) {}
void OverviewListsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto OverviewListsDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const -> QSize
{
    return QStyledItemDelegate::sizeHint(option, index);
}
EcuTreeDelegate::EcuTreeDelegate(QTreeView* view, QObject* parent) {}
void EcuTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto EcuTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const -> QSize
{
    return QStyledItemDelegate::sizeHint(option, index);
}
void EcuTreeDelegate::paintEcuCard(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index)
{
}
void EcuTreeDelegate::paintMessageRow(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
}
void EcuTreeDelegate::paintSignalRow(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
}
void EcuTreeDelegate::drawBadge(QPainter* painter, const QRect& rect, const QString& text,
                                const QColor& bg, const QColor& fg) const
{
}
MessagesSignalCardDelegate::MessagesSignalCardDelegate(QObject* parent) {}
void MessagesSignalCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto MessagesSignalCardDelegate::sizeHint(const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const -> QSize
{
    return QStyledItemDelegate::sizeHint(option, index);
}
void MessagesSignalCardDelegate::drawGridItem(QPainter* painter, const QRect& rect,
                                              const QString& label, const QString& value) const
{
}
void MessagesSignalCardDelegate::drawBadge(QPainter* painter, const QRect& rect,
                                           const QString& text) const
{
}
SignalTableDelegate::SignalTableDelegate(QObject* parent) {}
void SignalTableDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto SignalTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const -> QSize
{
    return QStyledItemDelegate::sizeHint(option, index);
};
}// namespace DbcFile