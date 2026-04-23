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

#include "fault_injector_type_delegate.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"

namespace FaultInjector {

FaultInjectorTypeDelegate::FaultInjectorTypeDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void FaultInjectorTypeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    painter->save();

    const QString label = index.data(Qt::UserRole).toString();

    QFont font = option.font;
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);

    const QFontMetrics fontMetrics(font);
    const int chipW = fontMetrics.horizontalAdvance(label) + spacing.spacingMd;
    const int chipH = fontMetrics.height() + spacing.spacingXs;
    const int cx = option.rect.left() + spacing.spacingXs;
    const int cy = option.rect.top() + (option.rect.height() - chipH) / 2;
    const QRect chipRect(cx, cy, chipW, chipH);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(colors.colorPrimary, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(chipRect, spacing.radiusXs, spacing.radiusXs);
    painter->setPen(colors.textPrimary);
    painter->drawText(chipRect, Qt::AlignCenter, label);

    painter->restore();
}

QSize FaultInjectorTypeDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    Q_UNUSED(index)
    const auto& spacing = THEME.spacing();
    QFont font = option.font;
    font.setPointSize(spacing.fontSizeXs);
    const QFontMetrics fm(font);
    return {option.rect.width(), fm.height() + spacing.spacingXs + spacing.spacingSm * 2};
}

}  // namespace FaultInjector