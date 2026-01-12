#ifndef CANBUSMANAGER_MONITORING_MODEL_HPP
#define CANBUSMANAGER_MONITORING_MODEL_HPP
#include <QAbstractItemModel>

#include "core/dto/can_dto.hpp"

#endif  // CANBUSMANAGER_MONITORING_MODEL_HPP

/**
 * @namespace Monitoring
 * @brief Contains data models and UI components for CAN signal monitoring.
 */
namespace Monitoring {

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
    /**
     * @brief Constructs the signal tree model.
     *
     * @param parent Optional Qt parent object.
     */
    explicit MonitoringModel(QObject* parent = nullptr);

    /**
     * @name QAbstractItemModel interface implementation
     * @{
     */

    /**
     * @brief Returns the model index for the given row and column.
     */
    [[nodiscard]] auto index(int row, int column,
                             const QModelIndex& parent) const -> QModelIndex override;

    /**
     * @brief Returns the parent index of a given model index.
     */
    [[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

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

    /**
     * @brief Returns the item flags for the given model index.
     *
     * Enables checkable and selectable behavior for frames and signals.
     */
    [[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

    /**
     * @brief Updates the data stored at the given index.
     *
     * Primarily used to handle check state changes initiated by the view.
     */
    auto setData(const QModelIndex& index, const QVariant& value, int role) -> bool override;

   private:
    /**
     * @struct SignalNode
     * @brief Internal representation of a single CAN signal within a frame.
     */
    struct SignalNode {
        Core::DbcCanSignal signal;
        Qt::CheckState checked;
    };

    /**
     * @struct FrameNode
     * @brief Internal representation of a CAN frame and its contained signals.
     */
    struct FrameNode {
        Core::DbcCanMessage message;
        QVector<SignalNode> allSignals;
        Qt::CheckState checked;
    };

    /**
     * @brief Collection of all tracked CAN frames.
     *
     * Each frame node contains its most recent data and associated signals.
     * The model maintains the hierarchical relationship required by the
     * tree view.
     */
    QVector<FrameNode> m_frames;  // contains last 1min/whatever of signal data
};
}  // namespace Monitoring