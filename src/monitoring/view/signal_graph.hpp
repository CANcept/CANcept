#pragma once
#include <qwt_plot_curve.h>

#include <QWidget>

#include "core/dto/can_dto.hpp"
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
class SignalGraph : public QWidget
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
    explicit SignalGraph(char messageId, std::string signalName, QWidget* parent = nullptr);

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
    void appendDataToGraph(Core::DbcCanSignal& signal);

    // Getters for identification in the list
    [[nodiscard]] auto getSignalName() const -> std::string
    {
        return m_signalName;
    }

    [[nodiscard]] auto getMessageId() const -> char
    {
        return m_messageId;
    }

    /**
     * @brief Removes the graph corresponding to the given signal.
     *
     * Used when a signal is deselected or removed from monitoring.
     *
     * @param signal Reference to the signal identifying the graph to remove.
     */
    void deleteGraph(Core::DbcCanSignal& signal);

   private:
    void setupPlot();

    QwtPlot* m_plot;
    QwtPlotCurve* m_curve;

    char m_messageId;
    std::string m_signalName;
};
}  // namespace Monitoring
