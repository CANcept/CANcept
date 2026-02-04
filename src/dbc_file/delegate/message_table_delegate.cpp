//
// Created by Adrian Rupp on 04.02.26.
//
#include "message_table_delegate.hpp"
namespace DbcFile {
MessageTableDelegate::MessageTableDelegate(QObject* parent) {}
void MessageTableDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}
QString MessageTableDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    return QStyledItemDelegate::displayText(value, locale);
}
}
