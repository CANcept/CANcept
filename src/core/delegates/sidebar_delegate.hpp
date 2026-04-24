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
#include <QStyledItemDelegate>
namespace Core {
/**
 * @class SidebarDelegate
 * @brief Custom item delegate for Sidebar list items.
 *
 * SidebarDelegate customizes the visual appearance and interaction
 * behavior of sidebar items. It applies theme-aware icon coloring,
 * adjusts item height, emphasizes the selected item, and provides
 * tooltips for disabled entries.
 *
 * Responsibilities:
 * - Tint icons based on selection state
 * - Bold font for selected items
 * - Enforce a minimum row height
 * - Show tooltip text for disabled items
 */
class SidebarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit SidebarDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    /**
     * @brief Sets the tooltip text shown for disabled items.
     *
     * The tooltip is displayed when the user hovers over
     * a disabled sidebar entry.
     *
     * @param toolTipText Tooltip text to display.
     */
    void setToolTipText(const QString& toolTipText);

    /**
     * @brief Initializes and customizes the style options for an item.
     *
     * This method modifies the default style options to:
     * - Render selected items with a bold font
     * - Recolor item icons according to the current theme
     *   and selection state
     *
     * @param option Style options to initialize.
     * @param index Model index of the item.
     */
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

    /**
     * @brief Returns the size hint for a sidebar item.
     *
     * Ensures a minimum item height to improve readability
     * and visual consistency.
     *
     * @param option Style options.
     * @param index Model index of the item.
     * @return Recommended item size.
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        -> QSize override;

    /**
     * @brief Handles help events such as tooltips.
     *
     * Displays a tooltip when hovering over disabled items.
     *
     * @param event Help event.
     * @param view View that received the event.
     * @param option Style options.
     * @param index Model index of the item.
     * @return True if the event was handled, false otherwise.
     */
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    /**Text to be displayed in possible tooltip when hovering disabled items.*/
    QString m_toolTipText;
};
}  // namespace Core
