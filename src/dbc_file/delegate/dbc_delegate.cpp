//
// Created by Adrian Rupp on 22.01.26.
//
#include "dbc_delegate.hpp"
namespace DbcFile {

DbcDelegate::DbcDelegate(QObject* parent) {}
void DbcDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}
QString DbcDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    return QStyledItemDelegate::displayText(value, locale);
}
}  // namespace DbcFile
