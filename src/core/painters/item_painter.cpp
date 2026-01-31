//
// Created by Adrian Rupp on 29.01.26.
//
#include "../painters/item_painter.hpp"

#include <QIcon>
#include <QPainter>

#include "core/theme/theme_manager.hpp"
namespace Core {
void ItemPainter::paintCardBackground(QPainter* p, const QRect& rect, bool selected)
{
    const auto& colors = ThemeManager::getInstance().colors();
    const auto& spacing = ThemeManager::getInstance().spacing();

    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    // Small margin, so the items don't stick together
    const QRect cardRect = rect.adjusted(spacing.itemCardGap, spacing.itemCardGap,
                                         -spacing.itemCardGap, -spacing.itemCardGap);

    const QColor bgColor = selected ? colors.surfaceSelected : colors.surfaceMain;
    const QColor borderColor = selected ? colors.borderStrong : colors.borderSubtle;

    p->setBrush(bgColor);
    p->setPen(QPen(borderColor, spacing.borderThin));
    p->drawRoundedRect(cardRect, spacing.radiusSm, spacing.radiusSm);

    p->restore();
}

void ItemPainter::paintIcon(QPainter* p, const QRect& rect, const QIcon& icon, bool selected)
{
    if (icon.isNull()) return;

    const auto& c = ThemeManager::getInstance().colors();
    const auto& s = ThemeManager::getInstance().spacing();

    // Position: left (12px margin) + vertically centered
    const int size = s.iconSm;
    const QRect target(rect.left() + s.itemCardPadding, rect.center().y() - (size / 2), size, size);

    // Tinting
    QPixmap pix = icon.pixmap(size, size);
    QPainter ip(&pix);
    ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
    ip.fillRect(pix.rect(), selected ? c.textPrimary : c.textSecondary);
    ip.end();

    p->drawPixmap(target, pix);
}

void ItemPainter::paintTitle(QPainter* painter, const QRect& rect, const QString& text, bool bold)
{
    const auto& c = ThemeManager::getInstance().colors();
    const auto& s = ThemeManager::getInstance().spacing();

    // Text starts 45px right of icon and 50px left of the badge
    const int textPadding = 2 * s.itemCardPadding + s.iconSm;
    const QRect textRect = rect.adjusted(textPadding, 0, -textPadding, 0);

    painter->save();
    painter->setPen(c.textPrimary);

    QFont f = painter->font();
    if (bold) f.setBold(true);
    f.setPointSize(s.fontSizeXs);
    painter->setFont(f);

    // "..." if text too long
    QString elided = painter->fontMetrics().elidedText(text, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

    painter->restore();
}

auto ItemPainter::paintBadge(QPainter* p, const QRect& rect, const QString& text,
                             const QIcon& icon) -> int
{
    if (text.isEmpty() && icon.isNull()) return 0;

    const auto& c = ThemeManager::getInstance().colors();
    const auto& s = ThemeManager::getInstance().spacing();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    // Calculate badge size
    int iconWidth = icon.isNull() ? 0 : s.iconXs;
    int padding = 10;

    QFont f = p->font();
    f.setPointSize(s.fontSizeBadge);
    p->setFont(f);

    int maxTextWidth = rect.width() / 2;
    auto elidedText = p->fontMetrics().elidedText(text, Qt::ElideRight, maxTextWidth);

    const int textWidth = p->fontMetrics().horizontalAdvance(elidedText);
    const int badgeWidth = s.badgePadding + iconWidth + (icon.isNull() ? 0 : s.badgePadding) +
                           textWidth + s.badgePadding;
    const int badgeHeight = s.badgeHeight;
    const QRect badgeRect(rect.right() - badgeWidth - s.itemCardPadding,
                          rect.center().y() - (badgeHeight / 2), badgeWidth, badgeHeight);

    // Badge background
    p->setBrush(c.badge);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(badgeRect, s.radiusXs, s.radiusXs);

    // Draw content
    // 1. Icon
    if (!icon.isNull())
    {
        const int y = badgeRect.top() + (badgeRect.height() - s.iconXs) / 2;

        const QRect iconTarget(badgeRect.left() + s.badgePadding, y, s.iconXs, s.iconXs);

        QPixmap ipix = icon.pixmap(s.iconXs, s.iconXs, QIcon::Normal, QIcon::On);

        p->drawPixmap(iconTarget, ipix);
    }

    // 2. Text
    p->setPen(c.textPrimary);
    const QRect textTarget =
        badgeRect.adjusted(icon.isNull() ? 0 : s.iconXs + s.badgePadding, 0, 0, 0);
    p->drawText(textTarget, Qt::AlignCenter, text);

    p->restore();
    return badgeWidth;
}

void ItemPainter::paintDetailText(QPainter* p, const QRect& rect, const QString& text,
                                  int badgeWidth)
{
    if (text.isEmpty()) return;

    const auto& c = ThemeManager::getInstance().colors();
    const auto& s = ThemeManager::getInstance().spacing();

    p->save();

    const int badgeSpace = 2 * s.itemCardPadding + badgeWidth;
    const QRect textRect = rect.adjusted(0, 0, -badgeSpace, 0);

    QFont f = p->font();
    f.setPointSize(s.fontSizeXs);
    p->setFont(f);

    p->setPen(c.textPrimary);
    p->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

    p->restore();
}
}  // namespace Core
