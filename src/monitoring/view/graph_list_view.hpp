#pragma once

#include <QAbstractItemDelegate>
#include <QScrollArea>
#include <QVBoxLayout>

#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "signal_graph.hpp"

class MonitoringDelegate;

/**
 * @namespace Monitoring
 * @brief Contains components responsible for visualizing CAN signals.
 */
namespace Monitoring {

/**
 * @class GraphListView
 * @brief Container widget managing multiple SignalGraph instances.
 *
 * GraphListView acts as a composite View that maintains a vertically
 * scrollable list of SignalGraph widgets. Each graph corresponds to a
 * selected CAN signal and visualizes its values over time.
 *
 * Responsibilities:
 * - Create and destroy SignalGraph widgets dynamically
 * - Route incoming signal data to the corresponding graph
 * - Manage layout and scrolling behavior
 *
 * This class contains no signal-processing logic and delegates all
 * visualization responsibilities to individual SignalGraph instances.
 */
class GraphListView : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs the graph list view widget.
     *
     * @param parent Optional Qt parent widget.
     */
    explicit GraphListView(MonitoringModel* m_model, MonitoringDelegate* m_delegate);

    /**
     * @brief Adds new signal to graph list.
     *
     * If a graph for the given signal already exists, nothing happens, otherwise the signal will
     * be appended to the list of plotted signals.
     *
     * @param messageId the id of the message the to be plotted signal belongs to
     * @param signalName the name of the to be plotted signal
     */
    void addGraph(const QString& messageId, const QString& signalName);

    void updateViewData();

    /**
     * @brief Removes the graph associated with the given signal.
     *
     * Called when a signal is deselected or no longer monitored.
     *
     * @param messageId the id of the message the plotted signal belongs to
     * @param signalName the name of the plotted signal
     */
    void deleteGraph(const QString& messageId, const QString& signalName);

   public slots:
    void signalChecked(bool enabled, const QString& messageId, const QString& signalId);

    void onDbcChange();

   private:
    /**
     * @brief Layout arranging SignalGraph widgets vertically.
     */
    QVBoxLayout* m_layout;

    /**
     * @brief Scroll area providing vertical scrolling for the graph list.
     */
    QScrollArea* m_scrollArea;

    /**
     * @brief Collection of active SignalGraph widgets.
     *
     * Each entry corresponds to one monitored CAN signal.
     */
    QList<SignalGraph*> m_signal_graphs;

    MonitoringModel* m_model;
};
}  // namespace Monitoring
