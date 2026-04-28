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
#include <QAbstractListModel>

#include "fault_injector/service/fault_handler.hpp"
#include "fault_injector/types/fault.hpp"

namespace FaultInjector {

/**
 * @brief Identifies the columns of the fault list table.
 */
enum class FaultListColumn : int { Type, Triggers, Effects, Strategy, Mutation };

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
     * @brief Removes the fault at the given row index.
     * @param row Zero-based row index. No-op if out of range.
     */
    void removeFault(int row);

    /**
     * @brief Builds a FaultHandler snapshot from the current fault list.
     *
     * Partitions the stored faults into raw and DBC lists and constructs
     * a FaultHandler ready to apply them.
     *
     * @return A FaultHandler populated with the current faults.
     */
    [[nodiscard]] auto get() -> FaultHandler;

    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   private:
    std::vector<Fault> m_faults;
};

}  // namespace FaultInjector