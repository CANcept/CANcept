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
    explicit CardListDelegate(int badgeRole, int detailRole = -1, const QIcon& badgeIcon = QIcon(),
                              QObject* parent = nullptr);
    ~CardListDelegate() override = default;

    /**
     * @brief Renders the card using ItemPainter helper.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Calculates the size.
     * @return A fixed height (e.g., 50px) and width based on the View's available space.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    int m_badgeRole;
    QIcon m_badgeIcon;
    int m_detailRole;
};
}  // namespace Core