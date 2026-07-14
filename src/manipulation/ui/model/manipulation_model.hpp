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

#include "manipulation/service/manipulation_handler.hpp"
#include "manipulation/types/manipulation.hpp"

namespace Manipulation {

/**
 * @brief Identifies the columns of the manipulation list table.
 */
enum class ManipulationListColumn : int { Type, Triggers, Effects, Strategy, Mutation };

/**
 * @brief Table model backing the manipulation view.
 *
 * Stores a list of manipulations (raw or DBC-based) and exposes them to a
 * QTableView via the standard Qt model/view interface.
 */
class ManipulationModel final : public QAbstractTableModel
{
    Q_OBJECT
   public:
    enum class Mode { Raw, Dbc };

    explicit ManipulationModel(QObject* parent = nullptr);

    /**
     * @brief Sets the active mode. Switching to Raw removes all DbcManipulation entries.
     */
    void setMode(Mode mode);

    [[nodiscard]] auto mode() const -> Mode
    {
        return m_mode;
    }

    /**
     * @brief Appends a manipulation to the model.
     * @param manipulation The manipulation to add (either RawManipulation or DbcManipulation).
     */
    void addManipulation(const ManipulationEntry& manipulation);

    /**
     * @brief Removes the manipulation at the given row index.
     * @param row Zero-based row index. No-op if out of range.
     */
    void removeManipulation(int row);

    /**
     * @brief Returns the current manipulation list, e.g. for serialization.
     */
    [[nodiscard]] auto entries() const -> const std::vector<ManipulationEntry>&
    {
        return m_manipulations;
    }

    /**
     * @brief Replaces the entire manipulation list, e.g. after deserializing one.
     */
    void setManipulations(std::vector<ManipulationEntry> manipulations);

    /**
     * @brief Builds a ManipulationHandler snapshot from the current manipulation list.
     *
     * Partitions the stored manipulations into raw and DBC lists and constructs
     * a ManipulationHandler ready to apply them.
     *
     * @return A ManipulationHandler populated with the current manipulations.
     */
    [[nodiscard]] auto get() -> ManipulationHandler;

    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   private:
    std::vector<ManipulationEntry> m_manipulations;
    Mode m_mode = Mode::Dbc;
};

}  // namespace Manipulation