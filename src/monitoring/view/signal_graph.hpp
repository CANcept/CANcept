#pragma once
#include <qwt_plot_curve.h>

#include <QVariant>

#include "core/dto/can_dto.hpp"
#include "core/widgets/card_widget.hpp"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"

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
     * @param messageId Id reference to the CAN signal used to initialize
     *               the graph. Ownership of the initial signal data is
     *               transferred to the internal model.
     * @param signalName signal name reference to the name of the CAN signal for
     * @param parent Optional Qt parent widget.
     */
    explicit SignalGraph(QString messageId, QString signalName, QWidget* parent = nullptr);

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
     * @param signalValues QVariant reference containing the latest signal value
     *               to be appended.
     * @param timestamps QVariant reference containing the timestamp of the new
     */
    void updateGraphData(QVariant timestamps, QVariant signalValues);

    // Getters for identification in the list
    [[nodiscard]] auto getSignalName() const -> QString
    {
        return m_signalName;
    }

    [[nodiscard]] auto getMessageId() const -> QString
    {
        return m_messageId;
    }

    [[nodiscard]] auto getContainer() const -> Core::CardWidget*
    {
        return m_container;
    }

    void setContainer(Core::CardWidget* container)
    {
        m_container = container;
    }

    void applyStyle();

   private:
    void setupPlot();

    QwtPlot* m_plot;
    QwtPlotCurve* m_curve;
    QwtPlotGrid* m_grid;
    Core::CardWidget* m_container;

    QString m_messageId;
    QString m_signalName;
};
}  // namespace Monitoring
