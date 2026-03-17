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

#include "sending_delegate.hpp"

#include <QDoubleSpinBox>
#include <QPainter>

#include "sending/constants.hpp"
#include "sending/model/sending_model.hpp"

namespace Sending {

SendingDelegate::SendingDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void SendingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    // For now, use default painting
    // Custom painting for message cards can be implemented later
    QStyledItemDelegate::paint(painter, option, index);
}

auto SendingDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&,
                                   const QModelIndex& index) const -> QWidget*
{
    // Check if this is a signal index (has internal pointer)
    if (index.internalPointer() != nullptr)
    {
        auto* editor = new QDoubleSpinBox(parent);
        editor->setDecimals(Constants::SIGNAL_VALUE_DECIMALS);
        editor->setRange(Constants::SIGNAL_VALUE_MIN, Constants::SIGNAL_VALUE_MAX);
        return editor;
    }

    return nullptr;
}

void SendingDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                   const QModelIndex& index) const
{
    if (const auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor))
    {
        model->setData(index, spinBox->value(), SendingModel::Role_SignalValue);
    }
}

}  // namespace Sending
