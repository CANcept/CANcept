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

#include "app_root/delegate/app_root_delegate.hpp"

#include <QIcon>
#include <QListView>
#include <QMetaType>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <algorithm>

#include "app_root/constants.hpp"
#include "core/macro/theme.hpp"

namespace AppRoot {

AppRootDelegate::AppRootDelegate(QObject* parent) : QAbstractItemDelegate(parent) {}

void AppRootDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    // Extract Data from Model
    const QString text = index.data(Qt::DisplayRole).toString();
    const QVariant iconVariant = index.data(Qt::DecorationRole);
    QIcon icon;
    if (iconVariant.isValid())
    {
        if (iconVariant.userType() == QMetaType::QIcon)
        {
            icon = *static_cast<const QIcon*>(iconVariant.constData());
        }
    }

    const int padding = THEME.spacing().spacingSm;
    const int radius = THEME.spacing().radiusMd;
    const QRect rect = option.rect.adjusted(1, 1, -1, -1);

    // Defines the states during hovering / when selected
    if (option.state & QStyle::State_Selected)
    {
        QPen borderPen(THEME.colors().surfaceForeground);
        borderPen.setWidth(THEME.spacing().borderThick);
        painter->setPen(borderPen);

        painter->setBrush(THEME.colors().surfaceMain);

        const QRectF borderRect = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
        painter->drawRoundedRect(borderRect, radius, radius);
    } else if (option.state & QStyle::State_MouseOver)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(THEME.colors().colorPrimaryHover);
        painter->drawRoundedRect(rect, radius, radius);
    }

    // Set up font for text measurement
    QFont font = painter->font();
    font.setPixelSize(THEME.spacing().fontSizeMd);
    font.setWeight(static_cast<QFont::Weight>((option.state & QStyle::State_Selected)
                                                  ? THEME.spacing().fontWeightMedium
                                                  : THEME.spacing().fontWeightNormal));
    painter->setFont(font);

    // Calculate dimensions for centering
    const int iconSize = rect.height() - (padding * 2);
    const int spacing = padding;
    const int textWidth = painter->fontMetrics().horizontalAdvance(text);
    const int contentWidth = iconSize + spacing + textWidth;
    const int startX = rect.left() + (rect.width() - contentWidth) / 2;

    // Draw icon centered
    const QRect iconRect(startX, rect.top() + padding, iconSize, iconSize);
    if (!icon.isNull())
    {
        const qreal dpr = painter->device()->devicePixelRatioF();
        QPixmap pixmap = icon.pixmap(iconRect.size(), dpr);

        QPainter iconPainter(&pixmap);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(pixmap.rect(), THEME.colors().surfaceForeground);
        iconPainter.end();

        painter->drawPixmap(iconRect, pixmap);
    }

    // Draw text
    painter->setPen(THEME.colors().textPrimary);
    const QRect textRect(startX + iconSize + spacing, rect.top(), textWidth, rect.height());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    painter->restore();
}

auto AppRootDelegate::sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> QSize
{
    const int baseHeight = THEME.spacing().fontSizeMd + (THEME.spacing().spacingLg * 2);

    // Calculate dynamic width based on available space and tab count
    const auto* view = qobject_cast<const QListView*>(option.widget);

    if (const int tabCount = index.model() ? index.model()->rowCount() : 1; view && tabCount > 0)
    {
        const int availableWidth = view->viewport()->width();
        const int tabWidth = std::max(availableWidth / tabCount, Constants::MIN_TAB_WIDTH);
        return {tabWidth, baseHeight};
    }

    return {Constants::MIN_TAB_WIDTH, baseHeight};
}

}  // namespace AppRoot