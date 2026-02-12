#pragma once

#include <memory>

// Core Interfaces
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"

// MVD Classes
#include "delegate/message_table_delegate.hpp"
#include "model/dbc_model.hpp"
#include "view/dbc_view.hpp"

namespace DbcFile {

/**
 * @class DbcComponent
 * @brief Tab component responsible for managing the DBC feature workflow.
 *
 * DbcComponent acts as the coordination layer between:
 *
 * - The DbcModel (data layer)
 * - The DbcView (UI layer)
 * - The application-wide event system (IEventBroker)
 *
 * Responsibilities include:
 *
 * - Initializing and connecting the model and view
 * - Subscribing to DBC parsing events
 * - Handling parse success and error events
 * - Providing extracted metadata (e.g., signal units, message senders)
 *   to the view for filtering purposes
 *
 * The component follows a lifecycle-based design via onStart() and onStop(),
 * ensuring that event subscriptions are properly managed.
 */
class DbcComponent : public Core::ITabComponent
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the DBC component.
     *
     * @details
     * Initializes the tab component and creates the associated view.
     * Signal and event connections are established during the component
     * lifecycle in `onStart()`.
     *
     * @param broker Reference to the central event broker.
     */
    explicit DbcComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     *
     * Cleans up component resources. Event subscriptions are
     * released in onStop().
     */
    ~DbcComponent() override = default;

    // --- Core::ITabComponent Interface Implementation ---

    /**
     * @brief Returns the widget representing this component's view.
     *
     * @return Pointer to the root widget of the DBC view.
     */
    auto getView() -> QWidget* override;

    // --- Core::ILifecycle Interface Implementation ---

    /**
     * @brief Called when the component becomes active.
     *
     * Establishes event subscriptions and connects view signals
     * to internal handlers.
     */
    void onStart() override;

    /**
     * @brief Called when the component is deactivated.
     *
     * Releases event subscriptions to avoid dangling callbacks
     * and ensures proper lifecycle cleanup.
     */
    void onStop() override;

    /**
     * @brief Extracts all unique signal units from a parsed DBC file to provide filtering options
     * for signals.
     *
     * Iterates over all message definitions and their signals,
     * collects unique unit strings, and returns them sorted
     * alphabetically (case-insensitive).
     *
     * @param event The parsed DBC event containing configuration data.
     * @return A sorted list of unique signal units.
     */
    static auto extractSignalUnits(const Core::DBCParsedEvent& event) -> QStringList;

    /**
     * @brief Extracts all unique message senders from a parsed DBC event to provide filtering
     * options for messages.
     *
     * Iterates over all message definitions and collects unique
     * transmitter names. The result is sorted alphabetically
     * (case-insensitive).
     *
     * @param event The parsed DBC event containing configuration data.
     * @return A sorted list of unique message sender names.
     */
    static auto extractSenders(const Core::DBCParsedEvent& event) -> QStringList;

   private slots:
    /**
     * @brief Handles file load requests initiated by the view.
     *
     * @details
     * Publishes a parse request event to the event broker containing
     * the selected DBC file path.
     *
     * @param filePath Path to the selected DBC file.
     */
    void onFileLoadRequested(const QString& filePath);

   private:
    /**
     * @brief Handles successful DBC parsing events.
     *
     * Updates the view state to reflect successful parsing,
     * enables navigation, and provides extracted metadata
     * (signal units and message senders) for filtering.
     *
     * @param event The parsed DBC event containing configuration data.
     */
    void onDbcParsed(const Core::DBCParsedEvent& event);

    /**
     * @brief Handles DBC parsing errors.
     *
     * @details
     * Displays an error message in the view and disables navigation
     * to prevent access to invalid data.
     *
     * @param event Event containing error details.
     */
    void onDbcParseError(const Core::DBCParseErrorEvent& event);

    /**
     * @brief Sets up signal-slot and event broker connections.
     *
     * @details
     * Subscribes to parse success and error events via the event broker
     * and connects view signals to their corresponding handlers.
     * This method is called during component startup.
     */
    void setupConnections();

    // --- Members ---

    /** @brief Ownership of the Data Model (Smart Model). */
    std::unique_ptr<DbcModel> m_model;

    /** @brief Ownership of the Composite View. */
    std::unique_ptr<DbcView> m_view;

    /** @brief Ownership of the Formatting Delegate (passed to View). */
    std::unique_ptr<MessageTableDelegate> m_delegate;

    /** @brief RAII Handle for success event subscription. */
    Core::Connection m_parseSuccessConn;

    /** @brief RAII Handle for error event subscription. */
    Core::Connection m_parseErrorConn;
};

}  // namespace DbcFile