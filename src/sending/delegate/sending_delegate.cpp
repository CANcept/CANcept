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

QWidget* SendingDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&,
                                       const QModelIndex& index) const
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
    auto* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    if (spinBox)
    {
        model->setData(index, spinBox->value(), SendingModel::Role_SignalValue);
    }
}

}  // namespace Sending
