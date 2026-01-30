//
// Created by Adrian Rupp on 29.01.26.
//
#pragma once
#include <QStyledItemDelegate>
namespace Core {
/**
 * @class CardListDelegate
 * @brief A generic delegate to render list items as "Cards" with Icon, Title, Detail Text, and
 * Badge.
 *
 * @details
 * This delegate does not hardcode specific data roles like "ECU Count".
 * Instead, the consuming View injects the Role IDs into the constructor.
 *
 * Visual Layout:
 * [ Icon ]  [ Title ]   [ Detail Text (optional) ]   [ Badge (Pill) ]
 */
class CardListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the delegate with specific data roles.
     *
     * @param badgeRole The model role used to fetch the text for the gray badge (right side).
     *                  If -1, no badge is drawn.
     * @param detailRole The model role for secondary text (middle/right).
     *                   If -1, no detail text is drawn.
     * @param badgeIcon Icon to put into the badge.
     * @param parent Parent object.
     */
    explicit CardListDelegate(int badgeRole,const QIcon& badgeIcon = QIcon(), int detailRole = -1,
                              QObject* parent = nullptr);
    ~CardListDelegate() override = default;

    /**
     * @brief Paints a card item.
     * @param painter QPainter used to draw the item.
     * @param option Style options describing the item state.
     * @param index The model index to retrieve data from.
     *
     * Draws the card background, icon, title, optional badge, and optional detail text.
     * Selection state is automatically handled for background and icon tinting.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Returns the preferred size of a card item.
     * @param option The style options for the item.
     * @param index The model index.
     * @return QSize representing the width and height of a card item.
     *
     * The size is determined by ThemeManager::spacing().itemCardWidth/Height.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    /** Model role to retrieve badge text */
    int m_badgeRole;

    /** Model role to retrieve detail text */
    QIcon m_badgeIcon;

    /** Icon displayed inside the badge */
    int m_detailRole;
};
}  // namespace Core