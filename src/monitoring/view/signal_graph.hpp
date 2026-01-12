#pragma once
#include <QWidget>

#include "monitoring/model/signal_graph_model.hpp"
#include "qwt_plot.h"

/**
 * @namespace Monitoring
 * @brief Contains components related to CAN signal visualization
 * and monitoring.
 */
namespace Monitoring {

/**
 * @class SignalGraph
 * @brief Widget responsible for visualizing a single CAN signal as a graph.
 *
 * SignalGraph encapsulates the graphical representation of a CAN signal over
 * time. It acts as the View layer for a SignalGraphModel, delegating all data
 * storage and processing responsibilities to the model while focusing solely
 * on rendering and UI interaction.
 *
 * Typical responsibilities:
 * - Display signal values over time
 * - Append incoming signal samples
 * - Update graphical representation on data changes
 * - Handle removal of a signal from visualization
 */
class SignalGraph : QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs a graph widget for a specific CAN signal.
     *
     * @param signal Rvalue reference to the CAN signal used to initialize
     *               the graph. Ownership of the initial signal data is
     *               transferred to the internal model.
     * @param parent Optional Qt parent widget.
     */
    explicit SignalGraph(const Core::DbcCanSignal&& signal, QWidget* parent = nullptr);

    /**
     * @brief Destroys the SignalGraph widget and releases associated
     * resources.
     */
    ~SignalGraph() override;

    /**
     * @brief Appends a new data sample to the graph.
     *
     * This method is typically called when a new CAN message for the
     * corresponding signal is received.
     *
     * @param signal Rvalue reference containing the latest signal value
     *               to be appended.
     */
    void appendDataToGraph(Core::DbcCanSignal&& signal);

    /**
     * @brief Removes the graph corresponding to the given signal.
     *
     * Used when a signal is deselected or removed from monitoring.
     *
     * @param signal Reference to the signal identifying the graph to remove.
     */
    void deleteGraph(Core::DbcCanSignal& signal);

   private:
    QwtPlot graph;

    /**
     * @brief Model holding the time-series data and graph state.
     *
     * Responsible for managing signal samples, scaling, and any
     * preprocessing required for visualization.
     */
    SignalGraphModel model;
};
}  // namespace Monitoring