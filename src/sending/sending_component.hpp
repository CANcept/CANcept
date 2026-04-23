/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QPointer>
#include <QWidget>
#include <chrono>
#include <memory>
#include <mutex>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_tab_component.hpp"
#include "delegate/sending_delegate.hpp"
#include "model/sending_model.hpp"
#include "view/sending_view.hpp"
#include "worker/repeated_producer_worker.hpp"
#include "worker/scheduled_item_queue.hpp"
#include "worker/sending_consumer_worker.hpp"

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
     * Initializes the model, view, delegate, and worker pipeline and wires them together.
     *
     * @param broker Event broker used for inter-component communication.
     */
    explicit SendingComponent(Core::IEventBroker& broker, Math::VariableRegistry* registry);

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
     * @brief Handles DBC config update on UI thread (queued from event broker thread).
     */
    void onDbcConfigReceived(const Core::DbcConfig& config);

    /**
     * @brief Handles DBC parse error on UI thread (queued from event broker thread).
     */
    void onDbcParseError() const;

   private:
    /**
     * @brief Initializes all signal/slot connections between components.
     */
    void setupConnections();

    /**
     * @brief Subscribes to broker events.
     */
    void setupBrokerSubscriptions();

    /**
     * @brief Checks if the CAN device is ready and updates the view overlay accordingly.
     */
    void checkDeviceReadiness() const;

    /**
     * @brief Publishes a raw CAN message in a worker thread.
     * Thread-safe: copies message before spawning thread.
     */
    void publishRawMessageAsync(const Core::RawCanMessage& message) const;

    /**
     * @brief Publishes a DBC CAN message in a worker thread.
     * Thread-safe: copies message before spawning thread.
     */
    void publishDbcMessageAsync(const Core::DbcCanMessage& message) const;

    /**
     * @brief Starts the producer/consumer pipeline at the given interval.
     */
    void startRepeatedSending(int intervalUs) const;

    /**
     * @brief Stops the producer/consumer pipeline.
     */
    void stopRepeatedSending() const;

    /**
     * @brief Sends the current message once (offloaded to a thread).
     */
    void sendOnce() const;

    /** @brief Global variable registry for expression variables. */
    Math::VariableRegistry* m_variableRegistry = nullptr;

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

    /** @brief RAII Handle for CAN driver change event subscription. */
    Core::Connection m_canDriverChangeConn;

    /** @brief Mutex protecting event broker access from multiple threads. */
    mutable std::mutex m_brokerMutex;

    /**
     * @brief Shared channel between producer and consumer.
     */
    std::unique_ptr<ScheduledItemQueue> m_queue;

    /** @brief Producer thread for repeated sending. */
    std::unique_ptr<RepeatedProducerWorker> m_repeatedWorker;

    /** @brief Consumer thread for actual sending. */
    std::unique_ptr<SendingConsumerWorker> m_consumerWorker;

    /** @brief Timestamp when the component started, used for diagnostics. */
    std::chrono::steady_clock::time_point m_startTime;

    /** @brief Cached state of device readiness to avoid redundant overlay updates. */
    mutable bool m_lastDeviceReadyState = true;

    /**
     * @brief Fault handler built once per send session (single send or repeated send start).
     */
    mutable std::shared_ptr<Core::IFaultHandler> m_activeFaultHandler;
};

}  // namespace Sending