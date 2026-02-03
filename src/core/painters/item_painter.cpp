//
// Created by Adrian Rupp on 29.01.26.
//
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

    const auto& c = THEME.colors();
    const auto& s = THEME.spacing();
    const int iconSize = s.IconSm;
    const int itemCardPadding = s.spacingMd;

    // Create rect for icon
    const QRect target(rect.left() + itemCardPadding, rect.center().y() - (iconSize / 2) + 1,
                       iconSize, iconSize);

    // Tinting icon
    QPixmap pix = icon.pixmap(iconSize, iconSize);
    QPainter ip(&pix);
    ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
    ip.fillRect(pix.rect(), selected ? c.textSecondary : c.textPrimary);
    ip.end();

    painter->drawPixmap(target, pix);
}

void ItemPainter::paintTitle(QPainter* painter, const QRect& rect, const QString& text, bool bold)
{
    const auto& c = THEME.colors();
    const auto& s = THEME.spacing();

    const int itemCardPadding = s.spacingMd;

    // [Padding] [icon] [Padding] [text]
    const int textPadding = itemCardPadding + s.IconSm + itemCardPadding;
    const QRect textRect = rect.adjusted(textPadding, 0, -textPadding, 0);

    painter->save();
    painter->setPen(c.textPrimary);

    QFont font = painter->font();
    if (bold) font.setBold(true);
    font.setPointSize(s.fontSizeXs);
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

    const auto& c = THEME.colors();
    const auto& s = THEME.spacing();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // --- Calculate badge size ---
    constexpr int badgeHeight = 18;

    constexpr int badgePadding = 5;
    const int iconWidth = icon.isNull() ? 0 : s.IconXs;
    const int maxTextWidth = rect.width() / 2;
    const auto elidedText = painter->fontMetrics().elidedText(text, Qt::ElideRight, maxTextWidth);
    const int textWidth = painter->fontMetrics().horizontalAdvance(elidedText);
    const int badgeWidth =
        badgePadding + iconWidth + (icon.isNull() ? 0 : badgePadding) + textWidth + badgePadding;
    const int itemCardPadding = s.spacingMd;
    const QRect badgeRect(rect.right() - badgeWidth - itemCardPadding,
                          rect.center().y() - (badgeHeight / 2) + 1, badgeWidth, badgeHeight);

    // --- Badge background ---
    painter->setBrush(c.surfaceSecondary);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(badgeRect, s.radiusXs, s.radiusXs);

    // --- Draw content ---
    // 1. Icon
    if (!icon.isNull())
    {
        const int yPos = badgeRect.top() + (badgeRect.height() - s.IconXs) / 2;

        const QRect iconTarget(badgeRect.left() + badgePadding, yPos, s.IconXs, s.IconXs);

        QPixmap pix = icon.pixmap(s.IconXs, s.IconXs, QIcon::Normal, QIcon::On);

        QPainter iconPainter(&pix);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(pix.rect(), c.textPrimary);
        iconPainter.end();

        painter->drawPixmap(iconTarget, pix);
    }

    // 2. Text
    QFont font = painter->font();
    font.setPointSize(s.fontSizeXs);
    painter->setFont(font);
    painter->setPen(c.textPrimary);
    const QRect textTarget =
        badgeRect.adjusted(icon.isNull() ? 0 : s.IconXs + badgePadding, 0, 0, 0);
    painter->drawText(textTarget, Qt::AlignCenter, text);

    painter->restore();
    return badgeWidth;
}

void ItemPainter::paintDetailText(QPainter* painter, const QRect& rect, const QString& text,
                                  const int badgeWidth)
{
    if (text.isEmpty()) return;

    const auto& c = THEME.colors();
    const auto& s = THEME.spacing();

    painter->save();

    const int itemCardPadding = s.spacingMd;
    const int badgeSpace = itemCardPadding + badgeWidth + itemCardPadding;
    const QRect textRect = rect.adjusted(0, 0, -badgeSpace, 0);

    QFont font = painter->font();
    font.setPointSize(s.fontSizeXs);
    painter->setFont(font);

    painter->setPen(c.textPrimary);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

    painter->restore();
}
}  // namespace Core
