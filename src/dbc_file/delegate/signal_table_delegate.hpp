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

#include <QPainter>
#include <QStyledItemDelegate>

namespace DbcFile {

/**
 * @brief Delegate for painting signal table rows with custom formatting.
 *
 * SignalTableDelegate renders each row in a QTableView representing CAN signals.
 * It customizes the appearance of:
 * - Message column (ID badge + name)
 * - Range column ([min, max])
 * - Length column (with optional unit)
 * - Unit column
 * - Standard text columns (with bolding for the signal name)
 *
 * The delegate uses Core::ItemPainter for consistent painting according to the theme.
 */
class SignalTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a SignalTableDelegate.
     * @param parent Optional parent QObject.
     */
    explicit SignalTableDelegate(QObject* parent = nullptr);

    /**
     * @brief Paints a cell in the signal table.
     * @param painter Painter object to perform drawing.
     * @param option Style options for the cell.
     * @param index Model index identifying the cell.
     *
     * Painting behavior depends on the column:
     * - Message column: ID badge + signal name
     * - Range column: "[min, max]" formatted text
     * - Length column: value with optional unit
     * - Unit column: default or model-provided unit
     * - Other columns: standard text (signal name bolded)
     *
     * Row backgrounds and separators are painted automatically.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Returns the preferred size of a table cell.
     * @param option Style options.
     * @param index Model index of the cell.
     * @return QSize representing cell width and height.
     *
     * Height is fixed using the application's theme spacing (HeightSm).
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
        -> QSize override;
};

}  // namespace DbcFile