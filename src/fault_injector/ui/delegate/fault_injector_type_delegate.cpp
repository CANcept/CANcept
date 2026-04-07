

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
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    painter->save();

    const QString label = index.data(Qt::UserRole).toString();
    const QRect chipRect = option.rect.adjusted(spacing.spacingSm, spacing.spacingSm,
                                                -spacing.spacingSm, -spacing.spacingSm);

    QFont font = option.font;
    font.setBold(true);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(colors.colorPrimary);
    painter->drawRoundedRect(chipRect, spacing.radiusXs, spacing.radiusXs);
    painter->setFont(font);
    painter->setPen(colors.textPrimary);
    painter->drawText(chipRect, Qt::AlignCenter, label);

    painter->restore();
}

QSize FaultInjectorTypeDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    Q_UNUSED(index)

    const auto& spacing = THEME.spacing();
    const QFontMetrics fontMetrics(option.font);
    return {option.rect.width(), fontMetrics.height() + spacing.spacingSm * 2};
}

}  // namespace FaultInjector