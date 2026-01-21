//
// Created by Adrian Rupp on 21.01.26.
//
#include "page_delegates.hpp"
namespace DbcFile {

void SidebarDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if (option->state & QStyle::State_Selected)
    {
        option->font.setBold(true);
    }
}

auto SidebarDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(std::max(s.height(), 40));
    return s;
}
OverviewListsDelegate::OverviewListsDelegate(QObject* parent) {}
void OverviewListsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
QSize OverviewListsDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
EcuTreeDelegate::EcuTreeDelegate(QTreeView* view, QObject* parent) {}
void EcuTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
QSize EcuTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
void EcuTreeDelegate::paintEcuCard(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
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
QSize MessagesSignalCardDelegate::sizeHint(const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
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
QSize SignalTableDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
};
}  // namespace DbcFile