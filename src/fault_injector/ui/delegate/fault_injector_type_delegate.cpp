

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