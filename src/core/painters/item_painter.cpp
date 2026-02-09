#include "../painters/item_painter.hpp"

#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/theme_manager.hpp"
namespace Core {
void ItemPainter::paintCardBackground(QPainter* painter, const QRect& rect, bool selected)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const int itemGap = spacing.spacingSm / 2;

    qreal penWidth = spacing.borderThin;
    qreal offset = penWidth / 2.0;

    QRectF cardRect(rect);
    cardRect.adjust(itemGap + offset, itemGap + offset, -(itemGap + offset), -(itemGap + offset));

    const QColor bgColor = selected ? colors.surfaceSelected : colors.surfaceMain;
    const QColor borderColor = selected ? colors.borderStrong.name(QColor::HexArgb)
                                        : colors.borderSubtle.name(QColor::HexArgb);

    painter->setBrush(bgColor);
    QPen pen(borderColor, penWidth);

    painter->setPen(pen);
    painter->drawRoundedRect(cardRect, spacing.radiusSm, spacing.radiusSm);

    painter->restore();
}

void ItemPainter::paintIcon(QPainter* painter, const QRect& rect, const QIcon& icon, bool selected)
{
    if (icon.isNull()) return;

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    const int iconSize = spacing.IconSm;
    const int itemCardPadding = spacing.spacingMd;

    // Create rect for icon
    const QRect target(rect.left() + itemCardPadding, rect.center().y() - (iconSize / 2) + 1,
                       iconSize, iconSize);

    // Tinting icon
    QPixmap pix = icon.pixmap(iconSize, iconSize);
    QPainter ip(&pix);
    ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
    ip.fillRect(pix.rect(), selected ? colors.textSecondary : colors.textPrimary);
    ip.end();

    painter->drawPixmap(target, pix);
}

void ItemPainter::paintTitle(QPainter* painter, const QRect& rect, const QString& text, bool bold)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const int itemCardPadding = spacing.spacingMd;

    // [Padding] [icon] [Padding] [text]
    const int textPadding = itemCardPadding + spacing.IconSm + itemCardPadding;
    const QRect textRect = rect.adjusted(textPadding, 0, -textPadding, 0);

    painter->save();
    painter->setPen(colors.textPrimary);

    QFont font = painter->font();
    if (bold) font.setBold(true);
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);

    // "..." if text too long
    const QString elided =
        painter->fontMetrics().elidedText(text, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

    painter->restore();
}

auto ItemPainter::paintBadge(QPainter* painter, const QRect& rect, const QString& text,
                             const QIcon& icon) -> int
{
    if (text.isEmpty() && icon.isNull()) return 0;

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // --- Calculate badge size ---
    const int badgeHeight = spacing.HeightXs;

    constexpr int badgePadding = 5;
    const int iconWidth = icon.isNull() ? 0 : spacing.IconXs;
    const int maxTextWidth = rect.width() / 2;
    const auto elidedText = painter->fontMetrics().elidedText(text, Qt::ElideRight, maxTextWidth);
    const int textWidth = painter->fontMetrics().horizontalAdvance(elidedText);
    const int badgeWidth =
        badgePadding + iconWidth + (icon.isNull() ? 0 : badgePadding) + textWidth + badgePadding;
    const int itemCardPadding = spacing.spacingMd;
    const QRect badgeRect(rect.right() - badgeWidth - itemCardPadding,
                          rect.center().y() - (badgeHeight / 2) + 1, badgeWidth, badgeHeight);

    // --- Badge background ---
    painter->setBrush(colors.surfaceSecondary);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(badgeRect, spacing.radiusXs / 2, spacing.radiusXs / 2);

    // --- Draw content ---
    // 1. Icon
    if (!icon.isNull())
    {
        const int yPos = badgeRect.top() + (badgeRect.height() - spacing.IconXs) / 2;

        const QRect iconTarget(badgeRect.left() + badgePadding, yPos, spacing.IconXs,
                               spacing.IconXs);

        QPixmap pix = icon.pixmap(spacing.IconXs, spacing.IconXs, QIcon::Normal, QIcon::On);

        QPainter iconPainter(&pix);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(pix.rect(), colors.textPrimary);
        iconPainter.end();

        painter->drawPixmap(iconTarget, pix);
    }

    // 2. Text
    QFont font = painter->font();
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);
    painter->setPen(colors.textPrimary);
    const QRect textTarget =
        badgeRect.adjusted(icon.isNull() ? 0 : spacing.IconXs + badgePadding, 0, 0, 0);
    painter->drawText(textTarget, Qt::AlignCenter, text);

    painter->restore();
    return badgeWidth;
}

void ItemPainter::paintDetailText(QPainter* painter, const QRect& rect, const QString& text,
                                  const int badgeWidth)
{
    if (text.isEmpty()) return;

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    painter->save();

    const int itemCardPadding = spacing.spacingMd;
    const int badgeSpace = itemCardPadding + badgeWidth + itemCardPadding;
    const QRect textRect = rect.adjusted(0, 0, -badgeSpace, 0);

    QFont font = painter->font();
    font.setPointSize(spacing.fontSizeXs);
    painter->setFont(font);

    painter->setPen(colors.textPrimary);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

    painter->restore();
}
}  // namespace Core
