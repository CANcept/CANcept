//
// Created by Adrian Rupp on 29.01.26.
//
#include "card_list_delegate.hpp"

#include <utility>

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"
#include "core/theme/theme_manager.hpp"
namespace Core {
CardListDelegate::CardListDelegate(const int badgeRole, QIcon badgeIcon, const int detailRole,
                                   QObject* parent)
    : QStyledItemDelegate(parent),
      m_badgeRole(badgeRole),
      m_detailRole(detailRole),
      m_badgeIcon(std::move(badgeIcon))
{
}

auto CardListDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize
{
    const auto& s = THEME.spacing();
    return {s.WidthMd, s.HeightMd};
}

void CardListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    // Alias
    using Painter = ItemPainter;

    // 1. Card Background
    const bool selected = option.state & QStyle::State_Selected;
    Painter::paintCardBackground(painter, option.rect, selected);

    // 2. Icon on the left
    // using standard Qt::DecorationRole
    const auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    Painter::paintIcon(painter, option.rect, icon, selected);

    // 3. Title on the left (right to icon)
    const QString title = index.data(Qt::DisplayRole).toString();
    Painter::paintTitle(painter, option.rect, title);

    // 4. Badge (right)
    int badgeWidth = 0;
    if (m_badgeRole >= 0)
    {
        // Get badge date
        const QString badgeText = index.data(m_badgeRole).toString();
        // Paint badge
        badgeWidth = Painter::paintBadge(painter, option.rect, badgeText, m_badgeIcon);
    }

    // 5. Detail text (only for signal cards)
    if (m_detailRole >= 0)
    {
        const QString detail = index.data(m_detailRole).toString();
        Painter::paintDetailText(painter, option.rect, detail, badgeWidth);
    }
}
}  // namespace Core
