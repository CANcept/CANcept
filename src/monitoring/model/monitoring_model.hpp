#pragma once
#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QMetaType>
#include <QTime>
#include <QVariant>
#include <atomic>
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
        Role_Unit
    };
    /**
     * @brief Constructs the signal tree model.
     *
     */
    explicit MonitoringModel();
    ~MonitoringModel() override
    {
        _execute = false;
        message_check_thread.join();
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
};
}  // namespace Monitoring

// Q_DECLARE_METATYPE(Monitoring::MessageTimestamp)