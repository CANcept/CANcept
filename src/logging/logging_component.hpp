#pragma once

#include <memory>

#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "logging/delegate/logging_delegate.hpp"
#include "logging/model/logging_model.hpp"
#include "logging/view/logging_view.hpp"

namespace Logging {

/**
 * @class LoggingComponent
 * @brief The Controller/Composition Root for the Logging module.
 *
 * @details
 * **RESPONSIBILITIES:**
 * - Implements the `Core::ITabComponent` interface to integrate into the AppRoot.
 * - **Lifecycle Management:** Creates and owns the Model, View, and Delegate.
 * - **Wiring:** Connects the View (Signals) to the System (EventBroker) and vice versa.
 *
 * **DATA FLOW:**
 * - User Input (View) -> Component -> EventBroker (Publish `ParseDbcRequestEvent`)
 * - System Event (`SignalLoggedEvent`) -> Model (Update Data)
 */
class LoggingComponent final : public Core::ITabComponent
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the component.
     *
     * @caller AppRoot (Module Loader).
     *
     * @details
     * Initializes the MVD stack, connects signals/slots, and subscribes to system events.
     *
     * @param broker Reference to the system-wide EventBroker.
     */
    explicit LoggingComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     * Cleans up the MVD stack. Broker connections are released automatically via RAII.
     */
    ~LoggingComponent() override;

    /**
     * @brief Returns the main widget (LoggingView) for display in the application window.
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
     * @brief Signal emitted when a new DBC configuration is available.
     * The Delegate will connect to this.
     */
    void dbcConfigurationChanged(const Core::DbcConfig& config);

    /** @brief Signal to model to record a raw hexadecimal frame */
    void receiveRawFrame(const Core::RawCanMessage& message);

    /** @brief Signal to delegate to record decoded DBC signal values */
    void receiveDbcSignals(const Core::DbcCanMessage& message);

   private slots:

    /**
     * @brief Triggered when the user selects a different CAN device/interface.
     * It also publishes the CanDriverChangeEvent.
     * @param deviceName The identifier of the newly selected hardware.
     */
    void onDeviceChanged(const std::string& deviceName);

    /**
     * @brief Activates Broker subscriptions.
     * Depending on user selection, it connects to Raw, DBC, or both.
     */
    void startLogging();

    /**
     * @brief Releases Broker subscriptions.
     * Calling .disconnect() on the Connection handles stops the data flow.
     */
    void stopLogging();

    /**
     * @brief Performs the actual file writing.
     * @param sessionId Unique ID of the log to export.
     * @param filePath Destination on the disk (from Delegate's file dialog).
     */
    void exportLogSession(const QString& sessionId, const QString& filePath);

    /**
     * @brief Triggered by the Delegate -> View when "Details" is clicked.
     * Fetches data from Model, builds the detail widget, and tells View to swap stack.
     */
    void onDetailRequested(const QModelIndex& index);

   private:
    /**
     * @brief The actual CSV "Instance" logic.
     * Logic for iterating through the Model's data and writing to the stream.
     */
    bool writeToCsv(const QString& sessionId, const QString& filePath);

    /**
     * @brief Helper to generate the detail widget for a specific session.
     */
    QWidget* createDetailWidget(const LogSession* session);

    /** @brief Ownership of the Data Model (Smart Model). */
    std::unique_ptr<LoggingModel> m_model;

    /** @brief Ownership of the Composite View. */
    std::unique_ptr<LoggingView> m_view;

    LoggingDelegate* m_delegate;
    std::unique_ptr<MessageSelectionDialog> m_dialog;

    /** @brief RAII Handle for raw message reveived event subscription. */
    Core::Connection m_rawMsgConn;

    /** @brief RAII Handle for dbc message received event subscription. */
    Core::Connection m_dbcMsgConn;

    /**
     * @brief RAII Handle for success event subscription.
     * This connects to the dbcConfigurationChanged signal in onStart()
     */
    Core::Connection m_parseSuccessConn;

    /** @brief RAII Handle for error event subscription. */
    Core::Connection m_parseErrorConn;
};

}  // namespace Logging