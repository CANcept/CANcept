/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QIcon>
#include <QStyledItemDelegate>

#include "core/macro/theme.hpp"
#include "core/painters/item_painter.hpp"

namespace Core {

/**
 * @brief Delegate for painting items as cards in a list view.
 *
 * CardListDelegate renders each row as a "card" with:
 * - Optional icon on the left (DecorationRole)
 * - Title (DisplayRole)
 * - Optional badge on the right
 * - Optional detail text next to the badge
 *
 * This delegate uses THEME spacing and colors for consistent styling.
 */
class CardListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a CardListDelegate.
     * @param badgeRole The model role used for the badge text. Set to -1 if unused.
     * @param badgeIcon Optional icon displayed inside the badge.
     * @param detailRole The model role used for the detail text. Set to -1 if unused.
     * @param parent Optional parent QObject.
     */
    explicit CardListDelegate(int badgeRole = -1, QIcon badgeIcon = QIcon(), int detailRole = -1,
                              QObject* parent = nullptr);

    /**
     * @brief Returns the preferred size of an item (row height).
     * @param option Style options.
     * @param index Model index (unused in this delegate).
     * @return QSize representing the row width and height.
     *
     * The height is taken from THEME spacing (HeightMd).
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;

    /**
     * @brief Paints an item as a card with icon, title, badge, and detail text.
     * @param painter The QPainter used for drawing.
     * @param option Style options for the item.
     * @param index Model index of the item to render.
     *
     * Painting order:
     * 1. Card background
     * 2. Left-aligned icon (if available)
     * 3. Right-aligned badge (text + optional icon)
     * 4. Detail text to the left of the badge (if available)
     * 5. Title text filling the remaining space
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

   private:
    int m_badgeRole;    ///< Model role for the badge text
    int m_detailRole;   ///< Model role for the detail text
    QIcon m_badgeIcon;  ///< Optional icon displayed in the badge
};

}  // namespace Core