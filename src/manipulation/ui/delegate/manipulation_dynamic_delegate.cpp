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

#include "manipulation_dynamic_delegate.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"
#include "manipulation/constants.hpp"
#include "manipulation_display.hpp"

namespace Manipulation {

ManipulationDynamicDelegate::ManipulationDynamicDelegate(
    std::function<QStringList(const QVariant&)> extractor, QObject* parent)
    : QStyledItemDelegate(parent), m_extractor(std::move(extractor))
{
}

void ManipulationDynamicDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QStringList labels = m_extractor(index.data(Qt::UserRole));
    painter->save();

    QFont font = option.font;
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);
    const QFontMetrics fm(font);

    const int chipH = fm.height() + spacing.spacingXs;
    const int cy = opt.rect.top() + (opt.rect.height() - chipH) / 2;
    int x = opt.rect.left() + spacing.spacingXs;
    const int visible = qMin(labels.size(), Constants::MAX_NUMBER_DYNAMIC_COMPONENTS);

    for (int i = 0; i < visible; ++i)
    {
        const int chipW = fm.horizontalAdvance(labels[i]) + spacing.spacingSm * 2;
        paintChip(painter, QRect(x, cy, chipW, chipH), labels[i]);
        x += chipW + spacing.spacingXs;
    }

    if (const int remaining = labels.size() - visible; remaining > 0)
    {
        const QRect overflowRect(x, cy, spacing.spacingXl, chipH);
        paintOverflow(painter, overflowRect, remaining);
    }

    painter->restore();
}

QSize ManipulationDynamicDelegate::sizeHint(const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
    Q_UNUSED(index)
    const auto& spacing = THEME.spacing();
    QFont font = option.font;
    font.setPointSize(spacing.fontSizeXs);
    const QFontMetrics fm(font);
    return {option.rect.width(), fm.height() + spacing.spacingXs + spacing.spacingSm * 2};
}

void ManipulationDynamicDelegate::paintChip(QPainter* painter, const QRect& rect,
                                            const QString& label) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    QFont font = painter->font();  // already set to fontSizeXs by paint()
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(colors.borderSubtle, 1));
    painter->setBrush(colors.surfaceSecondary);
    painter->drawRoundedRect(rect, spacing.radiusXs, spacing.radiusXs);
    painter->setPen(colors.textPrimary);
    painter->drawText(rect.adjusted(spacing.spacingXs, 0, -spacing.spacingXs, 0),
                      Qt::AlignVCenter | Qt::AlignCenter, label);
}

void ManipulationDynamicDelegate::paintOverflow(QPainter* painter, const QRect& rect,
                                                const int count) const
{
    const auto& colors = THEME.colors();
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(colors.textSecondary);
    painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("+ %1").arg(count));
}

}  // namespace Manipulation