/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logging_delegate.hpp"

#include <QIcon>
#include <QMouseEvent>
#include <QPainter>

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

        const auto paintIcon = [&](const QString& path, int drawX) {
            QPixmap px = QIcon(path).pixmap(spacing.IconSm, spacing.IconSm);
            QPainter px_painter(&px);
            px_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            px_painter.fillRect(px.rect(), colors.surfaceForeground);
            px_painter.end();
            painter->drawPixmap(QRect(drawX, y - px.height() / 2, px.width(), px.height()), px);
        };

        paintIcon(":/assets/icon/logging/logging_export.svg", x);
        x += spacing.IconSm + spacing.spacingMd;
        paintIcon(":/assets/icon/logging/logging_detail_view.svg", x);
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
        QRect exportRect(x,
                         option.rect.top() + option.rect.height() / 2 - THEME.spacing().IconSm / 2,
                         THEME.spacing().IconSm, THEME.spacing().IconSm);
        if (exportRect.contains(mouseEvent->pos()))
        {
            // Trigger export action
            emit const_cast<LoggingDelegate*>(this)->exportClicked(index);
            return true;
        }

        x += THEME.spacing().IconSm + THEME.spacing().spacingMd;

        // Detail/View button area
        QRect viewRect(x, option.rect.top() + option.rect.height() / 2 - THEME.spacing().IconSm / 2,
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
