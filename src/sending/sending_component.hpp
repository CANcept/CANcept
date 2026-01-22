#pragma once

#include <QWidget>
#include <memory>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"
#include "delegate/sending_delegate.hpp"
#include "model/sending_model.hpp"
#include "view/sending_view.hpp"

namespace Sending {
/**
 * @brief Tab component responsible for sending CAN frames.
 *
 * The SendingComponent represents the composition root of the
 * "Send CAN Signals" tab. It owns and wires together the corresponding
 * model, view, and delegate according to the MVC pattern.
 *
 * UI state is managed exclusively by the view, while configuration
 * and domain data are stored in the model. The delegate mediates
 * between user interaction and application logic.
 */
class SendingComponent final : public Core::ITabComponent
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the SendingComponent.
     *
     * Initializes the model, view, and delegate and wires them together.
     *
     * @param broker Event broker used for inter-component communication.
     */
    explicit SendingComponent(Core::IEventBroker& broker);

    /**
     * @brief Destructor.
     *
     * Ensures orderly shutdown of the tab component and its subcomponents.
     */
    ~SendingComponent() override;

    /**
     * @brief Called when the application starts/module is activated.
     */
    void onStart() override;

    /**
     * @brief Called when the application stops/module is deactivated.
     */
    void onStop() override;

    /**
     * @brief Returns the widget representing this tab.
     *
     * @return Pointer to the root QWidget of the SendingView.
     */
    auto getView() -> QWidget* override;

   signals:
    /**
     * @brief Signal emitted when a new DBC configuration is available.
     * The Delegate will connect to this.
     */
    void dbcConfigurationChanged(const Core::DbcConfig& config);

   private slots:

    /**
     * @brief Triggered when the user selects a different CAN device/interface.
     * It also publishes the CanDriverChangeEvent.
     * @param deviceName The identifier of the newly selected hardware.
     */
    void onDeviceChanged(const std::string& deviceName);

    /**
     * @brief Triggered when the user clicks 'Send Message' in Raw mode.
     * Publishes a SendCanMessageRawEvent to the broker.
     * @param message the raw message to be send on the selected device/channel
     */
    void onSendRawRequested(const Core::RawCanMessage& message);

    /**
     * @brief Triggered when the user clicks 'Send Message' in DBC mode.
     * Publishes a SendCanMessageDbcEvent to the broker.
     * @param message the dbc based message to be send on the selected device/channel
     */
    void onSendDbcRequested(const Core::DbcCanMessage& message);

   private:
    /**
     * @brief Initializes all signal/slot connections between components.
     */
    void setupConnections();

    /**
     * @brief Subscribes to broker events.
     */
    void setupBrokerSubscriptions();

    /** @brief Model holding CAN sending configuration and data */
    std::unique_ptr<SendingModel> m_model;

    /** @brief View responsible for rendering the sending UI */
    std::unique_ptr<SendingView> m_view;

    SendingDelegate* m_delegate;

    /**
     * @brief RAII Handle for success event subscription.
     * This connects to the dbcConfigurationChanged signal in onStart()
     */
    Core::Connection m_parseSuccessConn;

    /** @brief RAII Handle for error event subscription. */
    Core::Connection m_parseErrorConn;
};

}  // namespace Sending