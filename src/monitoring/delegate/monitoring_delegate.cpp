#include "monitoring_delegate.hpp"

#include <QApplication>
#include <QPainter>

#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

MonitoringDelegate::MonitoringDelegate(MonitoringModel* model) : QStyledItemDelegate(nullptr)
{
    m_model = model;
}

auto MonitoringDelegate::displayText(const QVariant& value, const QLocale& locale) const -> QString
{
    return QStyledItemDelegate::displayText(value, locale);
}

void MonitoringDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isMessage = !index.parent().isValid();
    QRect rect = option.rect.adjusted(5, 5, -5, -5);  // Add some padding between rows

    if (isMessage)
    {
        drawMessageNode(painter, rect, index);
    } else
    {
        rect.setLeft(rect.left() + 30);
        drawSignalLeaf(painter, rect, index);
    }

    painter->restore();
}

void MonitoringDelegate::drawMessageNode(QPainter* painter, const QRect& rect,
                                         const QModelIndex& index) const
{
    painter->setBrush(QColor("#F0F0F0"));
    painter->setPen(QPen(QColor("#C0C0C0"), 1));
    painter->drawRoundedRect(rect, 8, 8);

    QRect iconRect(rect.left() + 10, rect.top() + (rect.height() - 32) / 2, 32, 32);
    painter->drawPixmap(iconRect, QPixmap(":/path/to/placeholder.png"));

    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(rect.adjusted(50, 5, -50, -rect.height() / 2), Qt::AlignBottom,
                      index.data().toString());

    painter->setFont(QFont("Arial", 8));
    painter->setPen(Qt::gray);
    QString idText = QString("ID: 0x%1").arg(index.data(Qt::UserRole).toUInt(), 0, 16);
    painter->drawText(rect.adjusted(50, rect.height() / 2, -50, -5), Qt::AlignTop, idText);

    int signalCount = index.model()->rowCount(index);
    painter->setPen(Qt::blue);
    painter->drawText(rect.adjusted(0, 0, -40, 0), Qt::AlignRight | Qt::AlignVCenter,
                      QString::number(signalCount));

    QRect checkRect(rect.right() - 30, rect.top() + (rect.height() - 20) / 2, 20, 20);
    painter->drawRect(checkRect);
}

void MonitoringDelegate::drawSignalLeaf(QPainter* painter, const QRect& rect,
                                        const QModelIndex& index) const
{
    painter->setBrush(Qt::white);
    painter->setPen(QPen(QColor("#D0D0D0"), 1));
    painter->drawRoundedRect(rect, 6, 6);

    painter->setPen(Qt::black);
    painter->drawText(rect.adjusted(30, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
                      index.data().toString());

    QString value = index.data(Qt::UserRole + 1).toString();
    QString unit = "V";

    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(rect.adjusted(0, 5, -10, -rect.height() / 2),
                      Qt::AlignRight | Qt::AlignBottom, value);

    painter->setFont(QFont("Arial", 8));
    painter->setPen(Qt::darkGray);
    painter->drawText(rect.adjusted(0, rect.height() / 2, -10, -5), Qt::AlignRight | Qt::AlignTop,
                      unit);
}

auto MonitoringDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const -> QSize
{
    if (!index.parent().isValid())
    {
        return {200, 60};
    }
    return {200, 45};
}

}  // namespace Monitoring