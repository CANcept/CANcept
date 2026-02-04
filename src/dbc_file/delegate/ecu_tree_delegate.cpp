//
// Created by Adrian Rupp on 04.02.26.
//
#include "ecu_tree_delegate.hpp"

#include <QTreeView>

namespace DbcFile {
EcuTreeDelegate::EcuTreeDelegate(QTreeView* view, QObject* parent) {}
void EcuTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto EcuTreeDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
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
}