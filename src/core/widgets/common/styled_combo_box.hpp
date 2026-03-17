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

#include <QComboBox>
#include <QStyledItemDelegate>

namespace Core {

/**
 * @class StyledComboBoxDelegate
 * @brief Custom delegate for styling combo box items without borders.
 *
 * Handles the painting of combo box items with proper hover and selection states. This is necessary
 * since the standard delegate has inflexible styles.
 */
class StyledComboBoxDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit StyledComboBoxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

/**
 * @class StyledComboBox
 * @brief A styled combo box with rounded corners and theme-aware colors.
 *
 * Provides a modern combo box appearance that matches the application theme.
 */
class StyledComboBox final : public QComboBox
{
    Q_OBJECT

   public:
    explicit StyledComboBox(QWidget* parent = nullptr);
    ~StyledComboBox() override = default;

    /**
     * @brief Shows the combo box popup menu.
     *
     * Overrides the default behavior to ensure the dropdown always opens below
     * the combo box instead of trying to position itself above when near the
     * bottom of the screen.
     */
    void showPopup() override;

   signals:
    /**
     * @brief Emitted just before the popup menu is shown.
     *
     * Connect to this signal to refresh dropdown contents on-the-fly.
     */
    void aboutToShowPopup();

   protected:
    bool event(QEvent* event) override;

   private:
    void applyStyle();
};

}  // namespace Core
