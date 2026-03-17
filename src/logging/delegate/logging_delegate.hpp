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

#include "logging/model/logging_model.hpp"
#include "logging/view/components/message_selection_dialog.hpp"
#include "logging/view/logging_view.hpp"

namespace Logging {

/**
 * @class LoggingDelegate
 * @brief The controller logic that bridges the View, Model, and Modal Dialog.
 * * @details
 * It is responsible to route start/stop action, fulfill navigation and style.
 */
class LoggingDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the Delegate.
     */
    explicit LoggingDelegate(QObject* parent = nullptr);

    /**
     * @brief Renders signal tags and action buttons in their respective columns.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Handles mouse clicks to detect if an action button was pressed.
     * @details If a button area is clicked, it emits the corresponding signal.
     */
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

   signals:
    void exportClicked(const QModelIndex& index);
    void detailClicked(const QModelIndex& index);
};

}  // namespace Logging