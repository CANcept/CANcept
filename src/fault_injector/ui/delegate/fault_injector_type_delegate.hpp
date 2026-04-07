#pragma once
#include <QStyledItemDelegate>

namespace FaultInjector {

/**
 * @class FaultInjectorTypeDelegate
 * @brief This class handles the rendering of the column type inside a fault row
 */
class FaultInjectorTypeDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit FaultInjectorTypeDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

}  // namespace FaultInjector
