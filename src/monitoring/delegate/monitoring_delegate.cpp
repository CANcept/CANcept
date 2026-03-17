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

#include "monitoring_delegate.hpp"

#include <QApplication>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "monitoring/constants.hpp"
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

    QRect rect = option.rect.adjusted(5, 5, -5, -5);  // Add some padding between rows

    if (!index.parent().isValid())  // Message
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
    painter->setBrush(QColor(THEME.colors().surfacePrimary));
    painter->setPen(QPen(QColor(THEME.colors().borderSubtle), THEME.spacing().borderThin));
    painter->drawRoundedRect(rect, THEME.spacing().radiusSm, THEME.spacing().radiusSm);

    QRect iconRect(rect.left() + 10, rect.top() + (rect.height() - 32) / 2, THEME.spacing().IconSm,
                   THEME.spacing().IconSm);
    painter->drawPixmap(iconRect, QPixmap(Constants::ARROW_RIGHT_BUTTON_ICON_PATH));

    painter->setPen(THEME.colors().textPrimary);
    painter->setFont(QFont("Arial", 10, THEME.spacing().fontWeightBold));
    painter->drawText(rect.adjusted(50, 5, -50, -rect.height() / 2), Qt::AlignBottom,
                      index.data().toString());

    painter->setFont(QFont("Arial", 8));
    painter->setPen(THEME.colors().textSecondary);
    QString idText = QString("ID: 0x%1").arg(index.data(Qt::UserRole).toUInt(), 0, 16);
    painter->drawText(rect.adjusted(50, rect.height() / 2, -50, -5), Qt::AlignTop, idText);

    int signalCount = index.model()->rowCount(index);
    painter->setPen(THEME.colors().textPrimary);
    painter->drawText(rect.adjusted(0, 0, -40, 0), Qt::AlignRight | Qt::AlignVCenter,
                      QString::number(signalCount));

    QRect checkRect(rect.right() - 30, rect.top() + (rect.height() - 20) / 2, 20, 20);
    painter->drawRect(checkRect);
}

void MonitoringDelegate::drawSignalLeaf(QPainter* painter, const QRect& rect,
                                        const QModelIndex& index) const
{
    painter->setBrush(QColor(THEME.colors().surfacePrimary));
    painter->setPen(QPen(QColor(THEME.colors().borderSubtle), THEME.spacing().borderThin));
    painter->drawRoundedRect(rect, THEME.spacing().radiusSm, THEME.spacing().radiusSm);

    painter->setPen(THEME.colors().textPrimary);
    painter->drawText(rect.adjusted(30, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
                      index.data().toString());

    QString value = index.data(Qt::UserRole + 1).toString();

    painter->setFont(QFont("Arial", 10, THEME.spacing().fontWeightBold));
    painter->drawText(rect.adjusted(0, 5, -10, -rect.height() / 2),
                      Qt::AlignRight | Qt::AlignBottom, value);

    painter->setFont(QFont("Arial", 8));
    painter->setPen(THEME.colors().textSecondary);
    painter->drawText(rect.adjusted(0, rect.height() / 2, -10, -5), Qt::AlignRight | Qt::AlignTop,
                      nullptr);
}

auto MonitoringDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const -> QSize
{
    if (!index.parent().isValid())
    {
        return {THEME.spacing().WidthMd, THEME.spacing().HeightMd};
    }
    return {THEME.spacing().WidthMd, THEME.spacing().HeightMd};
}

}  // namespace Monitoring