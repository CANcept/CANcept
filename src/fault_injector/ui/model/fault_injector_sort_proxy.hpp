#pragma once
#include <QSortFilterProxyModel>

/**
 * @class FaultInjectorSortProxyModel
 * @brief This class is responsible to sort the faults of the model with RAW > DBC based.
 */
class FaultInjectorSortProxyModel final : public QSortFilterProxyModel
{
   public:
    explicit FaultInjectorSortProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
    {
    }

   protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
    {
        return left.data(Qt::UserRole + 1).toInt() < right.data(Qt::UserRole + 1).toInt();
    }
};