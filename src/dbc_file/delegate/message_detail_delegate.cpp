#include "message_detail_delegate.hpp"
namespace DbcFile {
MessagesDetailDelegate::MessagesDetailDelegate(QObject* parent) {}
void MessagesDetailDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}
auto MessagesDetailDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const -> QSize
{
    return QStyledItemDelegate::sizeHint(option, index);
}
void MessagesDetailDelegate::drawGridItem(QPainter* painter, const QRect& rect,
                                          const QString& label, const QString& value) const
{
}
void MessagesDetailDelegate::drawBadge(QPainter* painter, const QRect& rect,
                                       const QString& text) const
{
}
}  // namespace DbcFile
