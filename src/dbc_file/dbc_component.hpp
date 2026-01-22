//
// Created by Adrian Rupp on 25.12.25.
//
#pragma once

#include <memory>

// Core Interfaces
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"

// MVD Classes
#include "delegate/dbc_delegate.hpp"
#include "model/dbc_model.hpp"
#include "view/dbc_view.hpp"

// Forward declarations for Events
namespace Core {
}  // namespace Core

namespace DbcFile {

/**
 * @class DbcComponent
 * @brief The Controller/Composition Root for the DBC module.
 *
 * @details
 * **RESPONSIBILITIES:**
 * - Implements the `Core::ITabComponent` interface to integrate into the AppRoot.
 * - **Lifecycle Management:** Creates and owns the Model, View, and Delegate.
 * - **Wiring:** Connects the View (Signals) to the System (EventBroker) and vice versa.
 *
 * **DATA FLOW:**
 * - User Input (View) -> Component -> EventBroker (Publish `ParseDbcRequestEvent`)
 * - System Event (`DbcParsedEvent`) -> Component -> View (Unlock Navigation)
 * - System Event (`DbcParsedEvent`) -> Model (Update Data)
 */
class DbcComponent : public Core::ITabComponent
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
    explicit DbcComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     * Cleans up the MVD stack. Broker connections are released automatically via RAII.
     */
    ~DbcComponent() override;

    // --- Core::ITabComponent Interface Implementation ---

    /**
     * @brief Returns the main widget (DbcView) for display in the application window.
     * @caller AppRoot.
     */
    auto getView() -> QWidget* override;

    // --- Core::ILifecycle Interface Implementation ---

    /**
     * @brief Called when the application starts/module is activated.
     */
    void onStart() override;

    /**
     * @brief Called when the application stops/module is deactivated.
     */
    void onStop() override;

   private slots:
    /**
     * @brief Handles the file load request from the View.
     *
     * @caller DbcView::fileLoadRequested signal.
     *
     * @details
     * Creates a `ParseDbcRequestEvent` with the file path and publishes it
     * to the EventBroker. The CAN Handler will process this request.
     *
     * @param filePath The absolute path to the file.
     */
    void onFileLoadRequested(const QString& filePath) const;

   private:
    /**
     * @brief Callback: Triggered when a DBC file was successfully parsed.
     *
     * @caller EventBroker (lambda callback).
     *
     * @details
     * Used to update the UI state (e.g., call `m_view->setNavigationEnabled(true)`).
     * The Model updates its data automatically via its own subscription.
     */
    void onDbcParsed(const Core::DBCParsedEvent& event) const;

    /**
     * @brief Callback: Triggered when parsing failed.
     * @caller EventBroker (lambda callback).
     * @details Shows an error message to the user (e.g., via QMessageBox).
     */
    void onDbcParseError(const Core::DBCParseErrorEvent& event) const;

    /**
     * @brief Sets up internal connections between View signals and Component slots.
     * @caller Constructor.
     */
    void setupConnections();

    // --- Members ---

    /** @brief Ownership of the Data Model (Smart Model). */
    std::unique_ptr<DbcModel> m_model;

    /** @brief Ownership of the Composite View. */
    std::unique_ptr<DbcView> m_view;

    /** @brief Ownership of the Formatting Delegate (passed to View). */
    std::unique_ptr<DbcDelegate> m_delegate;

    /** @brief RAII Handle for success event subscription. */
    Core::Connection m_parseSuccessConn;

    /** @brief RAII Handle for error event subscription. */
    Core::Connection m_parseErrorConn;
};

}  // namespace DbcFile