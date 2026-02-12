#include "card_list_delegate.hpp"

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"

namespace Core {

CardListDelegate::CardListDelegate(int badgeRole, QIcon badgeIcon, int detailRole, QObject* parent)
    : QStyledItemDelegate(parent),
      m_badgeRole(badgeRole),
      m_badgeIcon(std::move(badgeIcon)),
      m_detailRole(detailRole)
{
}

auto CardListDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& /*index*/) const -> QSize
{
    return {option.rect.width(), THEME.spacing().HeightMd};
}

void CardListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    // 1. Setup & Layout constants
    const auto& spacing = THEME.spacing();
    bool selected = option.state & QStyle::State_Selected;

    // make card smaller than row to maintain gap between items
    int margin = spacing.spacingXs / 2;
    QRect cardRect = option.rect.adjusted(0, margin, 0, -margin);

    // -------------------------------------------------------------------------
    // 2. BACKGROUND (Card Style)
    // -------------------------------------------------------------------------
    ItemPainter::paintCard(painter, cardRect, selected);

    // -------------------------------------------------------------------------
    // 3. LOAD DATA
    // -------------------------------------------------------------------------
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    QString title = index.data(Qt::DisplayRole).toString();
    QString badgeText = (m_badgeRole >= 0) ? index.data(m_badgeRole).toString() : QString();
    QString detailText = (m_detailRole >= 0) ? index.data(m_detailRole).toString() : QString();

    // -------------------------------------------------------------------------
    // 4. ICON (left)
    // -------------------------------------------------------------------------
    int contentLeft = cardRect.left() + spacing.spacingMd;  // Start position for content

    if (!icon.isNull())
    {
        int iconSize = spacing.IconSm;
        // Vertically centered
        QRect iconRect(contentLeft, cardRect.center().y() - iconSize / 2 + 1, iconSize, iconSize);

        ItemPainter::paintIcon(painter, iconRect, icon, selected);

        // shift cursor to the right
        contentLeft += iconSize + spacing.spacingMd;
    }

    // -------------------------------------------------------------------------
    // 5. BADGE (right)
    // -------------------------------------------------------------------------
    int contentRight = cardRect.right() - spacing.spacingMd;

    if (!badgeText.isEmpty() || !m_badgeIcon.isNull())
    {
        // Measure badge size
        QSize badgeSize = ItemPainter::measureBadge(badgeText, m_badgeIcon);

        // calculate badge rect
        QRect badgeRect(contentRight - badgeSize.width(),
                        cardRect.center().y() - badgeSize.height() / 2 + 1, badgeSize.width(),
                        badgeSize.height());

        // Paint
        ItemPainter::paintBadge(painter, badgeRect, badgeText, m_badgeIcon);

        // Move cursor to the left
        contentRight -= (badgeSize.width() + spacing.spacingMd);
    }

    // -------------------------------------------------------------------------
    // 6. DETAIL TEXT (left of badge)
    // -------------------------------------------------------------------------
    if (!detailText.isEmpty())
    {
        int availableWidth = spacing.WidthSm;  // max width for detail text
        QRect detailRect(contentRight - availableWidth, cardRect.top(), availableWidth,
                         cardRect.height());

        painter->save();
        painter->setPen(THEME.colors().textPrimary);
        QFont f = painter->font();
        f.setPixelSize(spacing.fontSizeXs);
        painter->setFont(f);
        painter->drawText(detailRect, Qt::AlignRight | Qt::AlignVCenter, detailText);
        painter->restore();

        contentRight -= (availableWidth + spacing.spacingSm);
    }

    // -------------------------------------------------------------------------
    // 7. TITLE
    // -------------------------------------------------------------------------
    if (contentRight > contentLeft)
    {
        QRect titleRect(contentLeft, cardRect.top(), contentRight - contentLeft, cardRect.height());
        ItemPainter::paintText(painter, titleRect, title, true);
    }
}

}  // namespace Core