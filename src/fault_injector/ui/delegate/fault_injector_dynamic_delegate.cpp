#include "fault_injector_dynamic_delegate.hpp"

#include <QPainter>

#include "core/macro/theme.hpp"
#include "fault_display.hpp"
#include "fault_injector/constants.hpp"

namespace FaultInjector {

FaultInjectorDynamicDelegate::FaultInjectorDynamicDelegate(
    std::function<QStringList(const QVariant&)> extractor, QObject* parent)
    : QStyledItemDelegate(parent), m_extractor(std::move(extractor))
{
}

void FaultInjectorDynamicDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const auto& spacing = THEME.spacing();

    const QStringList labels = m_extractor(index.data(Qt::UserRole));
    painter->save();

    const int visible = qMin(labels.size(), Constants::MAX_NUMBER_DYNAMIC_COMPONENTS);
    int y = opt.rect.top() + spacing.spacingSm;

    for (int i = 0; i < visible; ++i)
    {
        const QRect chipRect(opt.rect.left() + spacing.spacingSm, y,
                             opt.rect.width() - spacing.spacingSm * 2, spacing.spacingLg);
        paintChip(painter, chipRect, labels[i]);
        y += spacing.spacingLg + spacing.spacingXs;
    }

    if (const int remaining = labels.size() - visible; remaining > 0)
    {
        const QRect overflowRect(opt.rect.left() + spacing.spacingSm, y,
                                 opt.rect.width() - spacing.spacingSm * 2, spacing.spacingLg);
        paintOverflow(painter, overflowRect, remaining);
    }

    painter->restore();
}

QSize FaultInjectorDynamicDelegate::sizeHint(const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const
{
    const auto& spacing = THEME.spacing();
    const QStringList labels = m_extractor(index.data(Qt::UserRole));

    const int visible = qMin(labels.size(), Constants::MAX_NUMBER_DYNAMIC_COMPONENTS);
    int h = spacing.spacingSm * 2;
    h += visible * (spacing.spacingLg + spacing.spacingXs);
    if (labels.size() > Constants::MAX_NUMBER_DYNAMIC_COMPONENTS)
        h += spacing.spacingLg + spacing.spacingXs;

    return {option.rect.width(), h};
}

void FaultInjectorDynamicDelegate::paintChip(QPainter* painter, const QRect& rect,
                                             const QString& label) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(colors.borderSubtle, 1));
    painter->setBrush(colors.surfaceSecondary);
    painter->drawRoundedRect(rect, spacing.radiusXs, spacing.radiusXs);
    painter->setPen(colors.textPrimary);
    painter->drawText(rect.adjusted(spacing.spacingXs, 0, -spacing.spacingXs, 0), Qt::AlignVCenter,
                      label);
}

void FaultInjectorDynamicDelegate::paintOverflow(QPainter* painter, const QRect& rect,
                                                 const int count) const
{
    const auto& colors = THEME.colors();
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(colors.colorPrimary);
    painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("+ %1").arg(count));
}

}  // namespace FaultInjector