//
// Created by Adrian Rupp on 25.12.25.
//
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
 * @brief The Controller/Composition Root for the DBC module.
 *
 * @details
 * RESPONSIBILITIES:
 * - Implements the `Core::ITabComponent` interface to integrate into the AppRoot.
 * - Lifecycle Management: Creates and owns the Model, View, and Delegate.
 * - Wiring: Connects the View (Signals) to the System (EventBroker) and vice versa.
 *
 * DATA FLOW:
 * - User Input (View) -> Component -> EventBroker (Publish `ParseDbcRequestEvent`)
 * - System Event (`DbcParsedEvent`) -> Component -> View (Unlock Navigation)
 * - System Event (`DbcParsedEvent`) -> Model (Update Data)
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
     * @brief Destroys the DBC component.
     */
    ~DbcComponent() override;

    // --- Core::ITabComponent Interface Implementation ---

    /**
     * @brief Returns the widget representing this component's view.
     *
     * @return Pointer to the root widget of the DBC view.
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
     * @brief Handles file load requests initiated by the view.
     *
     * @details
     * Publishes a parse request event to the event broker containing
     * the selected DBC file path.
     *
     * @param filePath Path to the selected DBC file.
     */
    void onFileLoadRequested(const QString& filePath) const;

   private:
    /**
     * @brief Handles successful DBC parsing.
     *
     * @details
     * Updates the view to indicate success and enables navigation
     * to the remaining pages of the DBC view.
     *
     * @param event Event containing the parsed DBC result.
     */
    void onDbcParsed(const Core::DBCParsedEvent& event) const;

    /**
     * @brief Handles DBC parsing errors.
     *
     * @details
     * Displays an error message in the view and disables navigation
     * to prevent access to invalid data.
     *
     * @param event Event containing error details.
     */
    void onDbcParseError(const Core::DBCParseErrorEvent& event) const;

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