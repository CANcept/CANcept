#include "logging_delegate.hpp"

#include <QApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QPaintDevice>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <iostream>

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "logging/model/logging_model.hpp"

namespace Logging {

// Constructs the delegate for custom painting of logging table items
LoggingDelegate::LoggingDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

// Custom painting for table cells (signals tags and action icons)
void LoggingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    if (!painter->isActive()) return;
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
        int y = option.rect.top() + option.rect.height() / 2;  // Center vertically

        QFont font = painter->font();
        font.setPixelSize(THEME.spacing().fontSizeXs);
        font.setWeight(QFont::Normal);
        painter->setFont(font);

        const auto& colors = THEME.colors();
        Core::ItemPainter::BadgeStyle idStyle;
        idStyle.background = colors.surfaceSecondary;
        idStyle.text = colors.textPrimary;
        idStyle.border = colors.borderSubtle;

        for (const QString& signal : signalsList)
        {
            QSize badgeSize = Core::ItemPainter::measureBadge(signal, QIcon());
            Core::ItemPainter::paintBadge(
                painter,
                QRect(x, y - badgeSize.height() / 2, badgeSize.width(), badgeSize.height()), signal,
                QIcon(), &idStyle);

            x += badgeSize.width() + THEME.spacing().spacingSm;
        }
    } else if (index.column() == LoggingModel::Col_Actions)
    {
        auto& colors = THEME.colors();
        auto& spacing = THEME.spacing();
        // Paint action icons
        int x = option.rect.left() + 5;
        int y = option.rect.top() + option.rect.height() / 2;
        painter->setPen(THEME.colors().textPrimary);
        const QIcon exportIcon(":/assets/icon/logging/logging_export.svg");
        exportIcon.paint(painter, QRect(x, y - spacing.IconSm / 2, spacing.IconSm, spacing.IconSm));
        // QPixmap exportPixmap = exportIcon.pixmap(QSize(spacing.IconSm, spacing.IconSm))
        //                            .scaled(spacing.IconSm, spacing.IconSm, Qt::KeepAspectRatio);
        // painter->drawPixmap(QRect(x, y - spacing.IconSm / 2, spacing.IconSm, spacing.IconSm),
        //                     exportPixmap);
        x += spacing.IconSm + spacing.spacingMd;

        const QIcon detailIcon(":/assets/icon/logging/logging_detail_view.svg");
        detailIcon.paint(painter, QRect(x, y - spacing.IconSm / 2, spacing.IconSm, spacing.IconSm));
        // QPixmap detailPixmap = detailIcon.pixmap(QSize(spacing.IconSm, spacing.IconSm));
        // painter->drawPixmap(QRect(x, y - spacing.IconSm / 2, spacing.IconSm, spacing.IconSm),
        //                    detailPixmap);
    } else
    {
        // Default rendering for other columns (Timestamp, Duration)
        QFont font = painter->font();
        font.setPixelSize(THEME.spacing().fontSizeMd);
        font.setWeight(QFont::Normal);
        painter->setFont(font);
        painter->setPen(THEME.colors().textPrimary);

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
        int x = option.rect.left() + 5;

        // Export button area
        QRect exportRect(x, option.rect.top() + option.rect.height() / 2 - THEME.spacing().IconSm,
                         THEME.spacing().IconSm, THEME.spacing().IconSm);
        if (exportRect.contains(mouseEvent->pos()))
        {
            // Trigger export action
            emit const_cast<LoggingDelegate*>(this)->exportClicked(index);
            return true;
        }

        x += THEME.spacing().IconSm + THEME.spacing().spacingMd;

        // Detail/View button area
        QRect viewRect(x, option.rect.top() + option.rect.height() / 2 - THEME.spacing().IconSm,
                       THEME.spacing().IconSm, THEME.spacing().IconSm);
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
