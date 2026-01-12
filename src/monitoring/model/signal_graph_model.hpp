#pragma once

#include <QObject>

#include "core/dto/can_dto.hpp"

/**
 * @namespace Monitoring
 * @brief Contains models and UI components for CAN signal visualization.
 */
namespace Monitoring {

/**
 * @class SignalGraphModel
 * @brief Model managing time-series data for a visualized CAN signal.
 *
 * SignalGraphModel represents the data layer for a single signal graph.
 * It stores incoming signal values, manages their lifecycle, and provides
 * the data required by the corresponding SignalGraph view.
 *
 * The model is intentionally UI-agnostic and communicates exclusively
 * through Qt signals and slots.
 */
class SignalGraphModel : public QObject
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs the signal graph model.
     *
     * @param parent Optional Qt parent object.
     */
    explicit SignalGraphModel(QObject* parent = nullptr);

   public slots:
    /**
     * @brief Adds a new signal sample to the model.
     *
     * Typically invoked when a new CAN message for the signal is received.
     *
     * @param signal Reference to the CAN signal containing the latest value.
     */
    void addSignal(Core::DbcCanSignal& signal);

    /**
     * @brief Removes all data associated with the given signal.
     *
     * Used when a signal is no longer monitored or the graph is destroyed.
     *
     * @param signal Reference to the CAN signal identifying the data to remove.
     */
    void removeSignal(Core::DbcCanSignal& signal);
};
}  // namespace Monitoring