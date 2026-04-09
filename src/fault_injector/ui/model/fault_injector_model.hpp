#pragma once
#include <QAbstractListModel>

#include "fault_injector/types/Fault.hpp"

namespace FaultInjector {

/**
 * @brief Identifies the columns of the fault list table.
 */
enum class FaultListColumn : int { Type, Triggers, Effects, Strategy };

/**
 * @brief Table model backing the fault injector view.
 *
 * Stores a list of faults (raw or DBC-based) and exposes them to a
 * QTableView via the standard Qt model/view interface.
 */
class FaultInjectorModel final : public QAbstractTableModel
{
    Q_OBJECT
   public:
    explicit FaultInjectorModel(QObject* parent = nullptr);

    /**
     * @brief Appends a fault to the model.
     * @param fault The fault to add (either RawFault or DbcFault).
     */
    void addFault(const Fault& fault);

    /**
     * @brief Builds a FaultHandler snapshot from the current fault list.
     *
     * Partitions the stored faults into raw and DBC lists and constructs
     * a FaultHandler ready to apply them.
     *
     * @return A FaultHandler populated with the current faults.
     */
    [[nodiscard]] FaultHandler get();

    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   private:
    std::vector<Fault> m_faults;
};

}  // namespace FaultInjector