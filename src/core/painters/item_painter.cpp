#include "item_painter.hpp"
#include "core/macro/theme.hpp"

namespace Core {

void ItemPainter::paintCard(QPainter* painter, const QRect& rect, bool selected)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    if (selected) {
        painter->setBrush(colors.surfaceSelected);
        painter->setPen(QPen(colors.colorPrimary, 1));
    } else {
        painter->setBrush(colors.surfaceMain);
        painter->setPen(QPen(colors.borderSubtle, 1));
    }

    painter->drawRoundedRect(rect, spacing.radiusSm, spacing.radiusSm);
    painter->restore();
}

auto ItemPainter::measureBadge(const QString& text, const QIcon& icon) -> QSize
{
    const auto& spacing = THEME.spacing();

    // Constants for badge layout
    const int paddingX = spacing.spacingXs;
    const int iconSize = spacing.IconSm;
    const int gap =  spacing.spacingXs;
    const int height = spacing.HeightXs;
    int width = paddingX * 2;

    // Measure width
    if (!text.isEmpty()) {
        QFont font;
        font.setPixelSize(spacing.fontSizeXs); // Badge Font Size
        QFontMetrics fm(font);
        width += fm.horizontalAdvance(text);
    }

    // Possibly add icon width
    if (!icon.isNull()) {
        width += iconSize;
        if (!text.isEmpty()) width += gap;
    }

    return {width, height};
}

void ItemPainter::paintBadge(QPainter* painter, const QRect& rect,
                             const QString& text, const QIcon& icon,
                             const BadgeStyle* style)
{
    const auto& spacing = THEME.spacing();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QFont badgeFont = painter->font();
    badgeFont.setPixelSize(spacing.fontSizeXs);
    painter->setFont(badgeFont);

    BadgeStyle badgeStyle = style ? *style : defaultBadgeStyle();

    // Calculate icon + text width
    int iconSize = (!icon.isNull()) ? spacing.IconXs : 0;
    int gap = (!icon.isNull() && !text.isEmpty()) ? spacing.spacingXs : 0;

    QFontMetrics fm(painter->font());
    int textWidth = (!text.isEmpty())
        ? fm.horizontalAdvance(text)
        : 0;
    int contentWidth = iconSize + gap + textWidth;

    // 1. Background
    painter->setBrush(badgeStyle.background);
    if (badgeStyle.border.isValid()) painter->setPen(badgeStyle.border);
    else painter->setPen(Qt::NoPen);

    // Round edges
    painter->drawRoundedRect(rect, spacing.radiusXs / 2, spacing.radiusXs / 2);

    // Layout within badge
    int paddingX = spacing.spacingXs;
    int availableWidth = rect.width() - paddingX * 2;

    int x = rect.left() + paddingX + (availableWidth - contentWidth) / 2;
    int centerY = rect.center().y();

    // 2. Icon
    if (!icon.isNull()) {
        QRect iconRect(x, centerY - iconSize/2 + 1, iconSize, iconSize);

        QPixmap pix = icon.pixmap(iconSize, iconSize);
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), badgeStyle.text); // Icon in text color
        p.end();

        painter->drawPixmap(iconRect, pix);
        x += iconSize + spacing.spacingXs;
    }

    // 3. Text
    if (!text.isEmpty()) {
        painter->setPen(badgeStyle.text);
        painter->setFont(badgeFont);

        QRect textRect(x, rect.top(), textWidth, rect.height());
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    painter->restore();
}

void ItemPainter::paintIcon(QPainter* painter, const QRect& rect, const QIcon& icon, bool selected)
{
    if (icon.isNull()) return;
    // paint icon vertically centered in given rect

    QSize iconSize(THEME.spacing().IconSm, THEME.spacing().IconSm);
    // scale if rect smaller than icon
    if(rect.width() < iconSize.width()) iconSize = rect.size();

    QPixmap pix = icon.pixmap(iconSize);
    QPainter p(&pix);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pix.rect(), selected ? THEME.colors().textPrimary : THEME.colors().textPrimary);
    p.end();

    // center
    int x = rect.center().x() - iconSize.width() / 2;
    int y = rect.center().y() - iconSize.height() / 2;

    painter->drawPixmap(x, y, pix);
}

void ItemPainter::paintText(QPainter* painter, const QRect& rect, const QString& text,
                            bool bold, const QColor& color, Qt::Alignment align, bool elide)
{
    if (text.isEmpty()) return;

    painter->save();
    const auto& c = THEME.colors();
    const auto& s = THEME.spacing();

    if (color.isValid()) {
        painter->setPen(color);
    } else {
        painter->setPen(c.textPrimary);
    }

    QFont f = painter->font();
    f.setPixelSize(s.fontSizeSm);
    f.setBold(bold);
    painter->setFont(f);

    if (elide) {
        QString elided = painter->fontMetrics().elidedText(text, Qt::ElideRight, rect.width());
        painter->drawText(rect, align, elided);
    } else {
        painter->drawText(rect, align, text);
    }

    painter->restore();
}
void ItemPainter::paintRow(QPainter* painter, const QRect& rect, bool selected, bool hovered)
{
    const auto& c = THEME.colors();
    QColor bgColor = c.surfaceMain;

    if (selected) {
        bgColor = c.surfaceSelected;
    } else if (hovered) {
        bgColor = c.surfaceHover;
    }

    painter->fillRect(rect, bgColor);

    painter->setPen(QPen(c.borderSubtle, 0));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
}
auto ItemPainter::defaultBadgeStyle() -> ItemPainter::BadgeStyle
{
    return {
        .background = THEME.colors().surfaceSecondary,
        .text       = THEME.colors().textPrimary,
        .border     = Qt::transparent
    };
}
} // namespace