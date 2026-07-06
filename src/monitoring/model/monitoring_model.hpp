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

#include <QAbstractItemModel>
#include <QList>
#include <QMetaType>
#include <QTime>
#include <QVariant>
#include <atomic>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
/**
 * @namespace Monitoring
 * @brief Contains data models and UI components for CAN signal monitoring.
 */

namespace Monitoring {
struct MessageTimestamp {
    QList<qreal> timestamps;
    std::vector<QList<qreal>> signalValues;
    QList<QString> signalNames;

    qreal windowStartNs = -1;
    std::vector<double> windowSum;
    std::vector<int> windowCount;
};

/**
 * @class MonitoringModel
 * @brief Hierarchical model representing CAN frames and their contained signals.
 *
 * MonitoringModel implements a two-level tree structure:
 * - Top level: CAN frames
 * - Child level: Signals belonging to each frame
 *
 * The model supports checkable items, allowing users to select or deselect
 * individual signals for visualization. Selection changes are communicated
 * via dedicated Qt signals, enabling loose coupling with visualization
 * components.
 */
class MonitoringModel final : public QAbstractItemModel
{
    Q_OBJECT
   public:
    enum MonitoringRoles {
        Role_Name = Qt::UserRole + 1,
        Role_ID,
        Role_ValueList,
        Role_LatestValue,
        Role_Unit,
        Role_Max,
        Role_Min
    };
    /**
     * @brief Constructs the signal tree model.
     *
     */
    explicit MonitoringModel();
    ~MonitoringModel() override
    {
        _execute = false;
        if (message_check_thread.joinable())
        {
            message_check_thread.join();
        }
    }

    /**
     * @brief Returns the model index for the given row and column.
     */
    [[nodiscard]] auto index(int row, int column,
                             const QModelIndex& parent) const -> QModelIndex override;

    /**
     * @brief Returns the parent index of a given model index.
     */
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;

    /**
     * @brief Returns the number of rows under the given parent.
     */
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns the number of columns for the given parent.
     */
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;

    /**
     * @brief Returns the data stored for the given index and role.
     */
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;

   public slots:

    /**
     * @brief Triggered when a new dbc decoded frame is incoming
     *
     * Adds the data to the batch
     *
     * @param message Reference to the received dbc decoded CAN message.
     */
    void onIncomingDbcFrame(const Core::DbcCanMessage& message);

    void onDbcChange(const Core::DbcConfig& config);

    void eraseOldData();

   private:
    std::unique_ptr<std::array<MessageTimestamp, 2048>> messageValues;
    std::optional<Core::DbcConfig> m_currentDbc;
    std::atomic<bool> _execute;
    std::atomic<bool> deleteOldData;
    std::thread message_check_thread;
    std::mutex m_dataMutex;
};
}  // namespace Monitoring