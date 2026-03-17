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

#include <QAbstractItemDelegate>

namespace AppRoot {

/**
 * @class AppRootDelegate
 * @brief Custom painter for the AppRootView shell.
 * @details
 * Handles the visual presentation of the TabBar and the Shell's background,
 * allowing for custom styling decoupled from the View's layout logic.
 */
class AppRootDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
   public:
    explicit AppRootDelegate(QObject* parent = nullptr);
    ~AppRootDelegate() override = default;

    /** * @brief Performs low-level painting of tab elements onto the view viewport.
     * @details Overrides the default delegate rendering. It queries the model via the
     * provided QModelIndex for TabRoles, uses the QStyleOptionViewItem to determine
     * widget states (Hover, Selected, Enabled), and executes drawing commands through
     * the QPainter primitive.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /** * @brief Calculates the geometry requirements for the view's layout engine.
     * @details Computes the bounding box dimensions based on font metrics, icon sizes,
     * and internal padding.
     * @return QSize The calculated width and height requirements.
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;
};

}  // namespace AppRoot
