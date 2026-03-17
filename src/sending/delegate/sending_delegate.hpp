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

#include <QObject>
#include <QStyledItemDelegate>

namespace Sending {

/**
 * @class SendingDelegate
 * @brief Delegate for custom rendering/editing in the Sending module.
 *
 * In strict MVD pattern, the Delegate only handles:
 * - Custom painting (paint)
 * - Custom editors (createEditor, setEditorData, setModelData)
 *
 * It does NOT create widgets or wire connections - that's the View's job.
 */
class SendingDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit SendingDelegate(QObject* parent = nullptr);
    ~SendingDelegate() override = default;

    /** @brief Custom painting for CAN message cards (if needed) */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /** @brief Creates custom editors for signal values (if needed) */
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    /**
     * @brief Writes data from the editor back to the Model.
     */
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};

}  // namespace Sending