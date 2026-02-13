#include "logging_delegate.hpp"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#include "logging/model/logging_model.hpp"

namespace Logging {

// Constructs the delegate for custom painting of logging table items
LoggingDelegate::LoggingDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

// Custom painting for table cells (signals tags and action icons)
void LoggingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Draw row border only for the first column to avoid overlapping borders
    if (index.column() == 0)
    {
        // Calculate the full row rect by getting the parent view
        QAbstractItemView* view =
            qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));

        if (view)
        {
            QRect rowRect = option.rect;
            rowRect.setLeft(view->visualRect(index.model()->index(index.row(), 0)).left());
            rowRect.setRight(view->visualRect(index.model()->index(
                                                  index.row(), index.model()->columnCount() - 1))
                                 .right());

            // Draw rounded border around entire row
            QPainterPath borderPath;
            borderPath.addRoundedRect(rowRect.adjusted(1, 1, -1, -1), 10, 10);
            painter->setPen(QPen(QColor(0, 0, 0, 26), 1));  // rgba(0, 0, 0, 0.1)
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(borderPath);

            // Draw hover/selection background with rounded corners
            if (option.state & QStyle::State_Selected)
            {
                QPainterPath bgPath;
                bgPath.addRoundedRect(rowRect.adjusted(1, 1, -1, -1), 10, 10);
                painter->fillPath(bgPath, QColor(0, 0, 0, 8));  // rgba(0, 0, 0, 0.03)
            } else if (option.state & QStyle::State_MouseOver)
            {
                QPainterPath bgPath;
                bgPath.addRoundedRect(rowRect.adjusted(1, 1, -1, -1), 10, 10);
                painter->fillPath(bgPath, QColor(0, 0, 0, 5));  // rgba(0, 0, 0, 0.02)
            }
        }
    }

    // Handle different columns
    if (index.column() == LoggingModel::Col_Signals)
    {
        // Paint signal tags
        QStringList signalsList = index.data(LoggingModel::SignalsListRole).toStringList();

        int x = option.rect.left() + 5;
        int y = option.rect.top() + (option.rect.height() - 30) / 2;  // Center vertically

        QFont font = painter->font();
        font.setPixelSize(16);
        font.setWeight(QFont::Normal);
        painter->setFont(font);

        for (const QString& signal : signalsList)
        {
            // Calculate tag width
            QFontMetrics fm(font);
            int textWidth = fm.horizontalAdvance(signal);
            int tagWidth = textWidth + 16;  // 8px padding on each side
            int tagHeight = 30;

            // Don't overflow the column
            if (x + tagWidth > option.rect.right() - 5) break;

            // Draw tag background (gray rounded rectangle)
            QPainterPath path;
            path.addRoundedRect(QRectF(x, y, tagWidth, tagHeight), 5, 5);
            painter->fillPath(path, QColor(226, 226, 226));  // #e2e2e2

            // Draw text
            painter->setPen(Qt::black);
            painter->drawText(QRect(x, y, tagWidth, tagHeight), Qt::AlignCenter, signal);

            x += tagWidth + 10;  // 10px spacing between tags
        }
    } else if (index.column() == LoggingModel::Col_Actions)
    {
        // Paint action icons
        int iconSize = 24;
        int spacing = 24;
        int x = option.rect.left() + 5;
        int y = option.rect.top() + (option.rect.height() - iconSize) / 2;

        // Export/Download icon (simplified as a rectangle with download arrow)
        QPainterPath downloadPath;
        downloadPath.moveTo(x + 8, y + 8);
        downloadPath.lineTo(x + 12, y + 8);
        downloadPath.lineTo(x + 12, y + 4);
        downloadPath.lineTo(x + 16, y + 10);
        downloadPath.lineTo(x + 4, y + 10);
        downloadPath.lineTo(x + 8, y + 4);
        downloadPath.lineTo(x + 8, y + 8);

        // Draw document outline
        painter->setPen(QPen(Qt::black, 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(x + 2, y + 2, 16, 20));
        painter->fillPath(downloadPath, Qt::black);

        x += iconSize + spacing;

        // Eye/View icon (simplified as an eye shape)
        QPainterPath eyePath;
        // Outer eye shape (ellipse)
        eyePath.addEllipse(QRectF(x + 2, y + 8, 20, 10));
        painter->drawPath(eyePath);

        // Pupil (filled circle)
        painter->setBrush(Qt::black);
        painter->drawEllipse(QRectF(x + 9, y + 10, 6, 6));
    } else
    {
        // Default rendering for other columns (Timestamp, Duration)
        QFont font = painter->font();
        font.setPixelSize(20);
        font.setWeight(QFont::Normal);
        painter->setFont(font);
        painter->setPen(Qt::black);

        QString text = index.data(Qt::DisplayRole).toString();
        painter->drawText(option.rect.adjusted(5, 0, -5, 0), Qt::AlignLeft | Qt::AlignVCenter,
                          text);
    }

    painter->restore();
}

// Handles mouse clicks on action icons (export and detail buttons)
bool LoggingDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                  const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease && index.column() == LoggingModel::Col_Actions)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        int iconSize = 24;
        int spacing = 24;
        int x = option.rect.left() + 5;

        // Export button area
        QRect exportRect(x, option.rect.top(), iconSize + 10, option.rect.height());
        if (exportRect.contains(mouseEvent->pos()))
        {
            // Trigger export action
            emit const_cast<LoggingDelegate*>(this)->exportClicked(index);
            return true;
        }

        x += iconSize + spacing;

        // Detail/View button area
        QRect viewRect(x, option.rect.top(), iconSize + 10, option.rect.height());
        if (viewRect.contains(mouseEvent->pos()))
        {
            // Trigger detail view action
            emit const_cast<LoggingDelegate*>(this)->detailClicked(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

}  // namespace Logging
