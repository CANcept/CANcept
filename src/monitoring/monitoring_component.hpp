#pragma once

#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"
#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "monitoring/view/monitoring_view.hpp"

/**
 * @namespace Monitoring
 * @brief Contains UI components related to CAN signal monitoring and
 * visualization.
 */
namespace Monitoring {

/**
 * @class MonitoringComponent
 * @brief UI tab component responsible for monitoring CAN signals and
 * managing their visualization.
 *
 * MonitoringComponent integrates a hierarchical signal tree with a graph list
 * view to allow users to inspect available CAN signals and select signals
 * for graphical visualization. The component subscribes to system events
 * via the event broker and acts as a composition root for its internal
 * models, views, and delegates.
 *
 * Architectural roles:
 * - UI tab entry (via ITabComponent)
 * - Event consumer / dispatcher (via IEventBroker)
 * - MVC coordinator for signal models and views
 */
class MonitoringComponent final : public Core::ITabComponent
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs the Monitoring tab.
     *
     * @param broker Pointer to the global event broker used for subscribing
     *               to and emitting application-wide events.
     */
    explicit MonitoringComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     *
     * Ensures orderly shutdown of the tab component and its subcomponents.
     */
    ~MonitoringComponent() override;

    /**
     * @brief Returns the main widget (MonitoringView) for display in the application window.
     * @caller AppRoot.
     */
    auto getView() -> QWidget* override;

    /**
     * @brief Called when the application starts/module is activated.
     */
    void onStart() override;

    /**
     * @brief Called when the application stops/module is deactivated.
     */
    void onStop() override;

   signals:
    /**
     * @brief Signal emitted when a new dbcConfiguration is available.
     * The available ECUs and signals refresh, no signal is selected for plotting.
     */
    void dbcConfigurationChanged();

    /**
     * @brief Updates the message data when a CAN frame is received.
     *
     * Adds new frames or updates existing ones and refreshes the signal
     * values associated with the frame.
     *
     * @param message Reference to the received CAN message.
     */
    void frameReceived(Core::DbcCanMessage& message);

   private slots:
    /**
     * @brief Triggered when the user selects a different CAN interface.
     * It also publishes the CanDriverChangeEvent.
     * @param deviceName The identifier of the newly selected hardware.
     */
    void onSourceChanged(const std::string& deviceName);

    /**
     * @brief Triggered when the user checks a signal to display in a graph
     * Notifies GraphListView to plot the checked signal in a new Graph.
     * @param messageId the id of the message the checked signal belongs to
     * @param signalName the name of the checked signal
     */
    void onSignalChecked(char messageId, const std::string& signalName);

    /**
     * @brief Triggered when the user unchecks a signal currently checked (therefor plotted in a
     * graph) Notifies GraphListView to erase graph of the checked signal and discontinue the
     * plotting
     * @param messageId the id of the message the unchecked signal belongs to
     * @param signalName the name of the unchecked signal
     */
    void onSignalUnchecked(char messageId, const std::string& signalName);

   private:
    /** @brief Model holding CAN sending configuration and data */
    std::unique_ptr<MonitoringModel> m_model;

    /** @brief View responsible for rendering the sending UI */
    std::unique_ptr<MonitoringView> m_view;

    /** @brief Delegate handling user actions and coordination */
    std::unique_ptr<MonitoringDelegate> m_delegate;

    /**
     * @brief RAII Handle for success event subscription.
     * This connects to the dbcConfigurationChanged signal in onStart()
     */
    Core::Connection m_parseSuccessConn;

    /** @brief RAII Handle for error event subscription. */
    Core::Connection m_parseErrorConn;
};
}  // namespace Monitoring