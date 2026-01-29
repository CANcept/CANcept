//
// Created by Adrian Rupp on 29.01.26.
//
#include "item_painter.hpp"

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

    // Small margin, so the items dont stick together
    QRect cardRect = rect.adjusted(2, 2, -2, -2);

    QColor bgColor = selected ? colors.surfaceSelected : colors.surfaceMain;
    QColor borderColor = selected ? colors.borderStrong : colors.borderSubtle;

    p->setBrush(bgColor);
    p->setPen(QPen(borderColor, spacing.borderThin));
    p->drawRoundedRect(cardRect, spacing.radiusSm, spacing.radiusSm);

    p->restore();
}

void ItemPainter::paintIcon(QPainter* p, const QRect& rect, const QIcon& icon, bool selected)
{
    if (icon.isNull()) return;

    const auto& colors = ThemeManager::getInstance().colors();
    const auto& spacing = ThemeManager::getInstance().spacing();

    // Position: left (12px margin) + vertically centered
    int size = spacing.iconSm;
    QRect target(rect.left() + 12, rect.center().y() - (size / 2), size, size);

    // Tinting
    QPixmap pix = icon.pixmap(size, size);
    QPainter ip(&pix);
    ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
    ip.fillRect(pix.rect(), selected ? colors.textPrimary : colors.textSecondary);
    ip.end();

    p->drawPixmap(target, pix);
}

void ItemPainter::paintTitle(QPainter* p, const QRect& rect, const QString& text, bool bold)
{
    const auto& c = ThemeManager::getInstance().colors();

    // Text starts 45px right of icon and 50px left of the badge
    QRect textRect = rect.adjusted(45, 0, -50, 0);

    p->save();
    p->setPen(c.textPrimary);

    QFont f = p->font();
    if (bold) f.setBold(true);
    p->setFont(f);

    // "..." if text too long
    QString elided = p->fontMetrics().elidedText(text, Qt::ElideRight, textRect.width());
    p->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

    p->restore();
}

void ItemPainter::paintBadge(QPainter* p, const QRect& rect, const QString& text, const QIcon& icon)
{
    if (text.isEmpty() && icon.isNull()) return;

    const auto& colors = ThemeManager::getInstance().colors();
    const auto& spacing = ThemeManager::getInstance().spacing();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    // Calculate badge size
    int iconWidth = icon.isNull() ? 0 : spacing.iconXs;
    int padding = 10;

    QFont f = p->font();
    f.setPointSize(spacing.fontSizeXs);
    p->setFont(f);

    int textWidth = p->fontMetrics().horizontalAdvance(text);
    int badgeWidth = padding + iconWidth + (icon.isNull() ? 0 : 4) + textWidth + padding;
    int badgeHeight = spacing.badgeHeight;
    QRect badgeRect(rect.right() - badgeWidth - 10, rect.center().y() - (badgeHeight / 2),
                    badgeWidth, badgeHeight);

    // Badge background
    p->setBrush(colors.badge);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(badgeRect, spacing.radiusXs, spacing.radiusXs);

    // Draw content
    // 1. Icon
    if (!icon.isNull())
    {
        QRect iconTarget(badgeRect.left() + 6, badgeRect.center().y() - 8, spacing.iconXs,
                         spacing.iconXs);
        QPixmap ipix = icon.pixmap(spacing.iconXs, spacing.iconXs);
        p->drawPixmap(iconTarget, ipix);
    }

    // 2. Text
    p->setPen(colors.textPrimary);
    int textX = badgeRect.left() + (icon.isNull() ? 0 : 20);
    // Rechteck für Text (Restplatz)
    QRect textTarget = badgeRect.adjusted(icon.isNull() ? 0 : 20, 0, 0, 0);
    p->drawText(textTarget, Qt::AlignCenter, text);

    p->restore();
}
}  // namespace Core
