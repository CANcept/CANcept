//
// Created by Adrian Rupp on 29.01.26.
//
#include "card_list_delegate.hpp"

#include "core/theme/theme_manager.hpp"
#include "core/ui/painters/item_painter.hpp"
namespace Core {
CardListDelegate::CardListDelegate(const int badgeRole, const int detailRole,
                                   const QIcon& badgeIcon, QObject* parent)
    : QStyledItemDelegate(parent), m_badgeRole(badgeRole), m_detailRole(detailRole)
{
}

QSize CardListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto& spacing = ThemeManager::getInstance().spacing();

    return QSize(option.rect.width(), spacing.itemCardHeight);
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
    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    Painter::paintIcon(painter, option.rect, icon, selected);

    // 3. Title on the left (right to icon)
    const QString title = index.data(Qt::DisplayRole).toString();
    Painter::paintTitle(painter, option.rect, title);

    // 4. Badge (Rechts)
    if (m_badgeRole >= 0) {
        // Zahl holen (z.B. "4")
        QString badgeText = index.data(m_badgeRole).toString();

        // m_badgeIcon ist z.B. das "Message"-Icon für ECUs
        // oder NULL für Signale
        Painter::paintBadge(painter, option.rect, badgeText, m_badgeIcon);
    }

    // 5. Detail text (only for signal cards)
    if (m_detailRole >= 0)
    {
        QString detail = index.data(m_detailRole).toString();
        //
        // Painter::paintDetailText(painter, option.rect, detail);
    }
}
}  // namespace Core
