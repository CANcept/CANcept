#include "monitoring_delegate.hpp"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

namespace Monitoring {

MonitoringDelegate::MonitoringDelegate() : QStyledItemDelegate(nullptr) {}

void MonitoringDelegate::setModel(QAbstractItemModel* model)
{
    // If you need to track model-specific changes in the delegate logic
    Q_UNUSED(model);
}

auto MonitoringDelegate::displayText(const QVariant& value, const QLocale& locale) const -> QString
{
    // We override paint, so displayText is mostly used for tooltips/accessibility
    return QStyledItemDelegate::displayText(value, locale);
}

void MonitoringDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 1. Setup Geometry
    QRect rect = option.rect.adjusted(5, 2, -5, -2);  // Margin
    bool isSelected = option.state & QStyle::State_Selected;
    bool isFrame = !index.parent().isValid();  // Top level are frames

    // 2. Draw Background Card
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);

    QColor bgColor = isSelected ? QColor(45, 125, 250, 40) : QColor(250, 250, 250);
    QColor borderColor = isSelected ? QColor(45, 125, 250) : QColor(220, 220, 220);

    painter->fillPath(path, bgColor);
    painter->setPen(QPen(borderColor, 1));
    painter->drawPath(path);

    // 3. Draw Icon Placeholder (Left side)
    QRect iconRect(rect.left() + 10, rect.top() + (rect.height() - 24) / 2, 24, 24);
    painter->setBrush(isFrame ? Qt::darkGray : Qt::gray);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(iconRect);

    // 4. Draw Text (Center)
    QString text = index.data(Qt::DisplayRole).toString();
    painter->setPen(Qt::black);
    QFont font = painter->font();
    if (isFrame) font.setBold(true);
    painter->setFont(font);

    QRect textRect = rect.adjusted(45, 0, -50, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    // 5. Draw Checkbox (Handled by the base class usually, but we can custom draw)
    if (index.model()->flags(index) & Qt::ItemIsUserCheckable)
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
        QRect checkRect(rect.right() - 30, rect.top() + (rect.height() - 20) / 2, 20, 20);

        // Use standard Qt primitive to draw the checkbox
        QStyleOptionButton checkbox;
        checkbox.rect = checkRect;
        checkbox.state = option.state | QStyle::State_Enabled;
        if (state == Qt::Checked)
            checkbox.state |= QStyle::State_On;
        else
            checkbox.state |= QStyle::State_Off;

        qApp->style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
    }

    painter->restore();
}

auto MonitoringDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const -> QSize
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    // Return a fixed height for our "cards"
    return {200, 45};
}

}  // namespace Monitoring