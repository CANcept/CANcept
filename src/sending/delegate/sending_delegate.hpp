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