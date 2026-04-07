#pragma once
#include <QAbstractListModel>

#include "fault_injector/types/Fault.hpp"

namespace FaultInjector {

enum class FaultListColumn : int { Type, Triggers, Effects, Strategy };

class FaultInjectorModel final : public QAbstractTableModel
{
    Q_OBJECT
   public:
    explicit FaultInjectorModel(QObject* parent = nullptr);

    void addFault(const Fault& fault);

    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   private:
    std::vector<Fault> m_faults;
};

}  // namespace FaultInjector