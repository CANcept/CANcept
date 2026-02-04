//
// Created by Adrian Rupp on 04.02.26.
//
#include "signal_table_delegate.hpp"
namespace DbcFile {
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
}
