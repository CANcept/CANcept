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

#include "sending_component.hpp"

#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>

#include "constants.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "core/macro/console_logging.hpp"

namespace Sending {

SendingComponent::SendingComponent(Core::IEventBroker& broker)
    : ITabComponent(broker, QString::fromStdString(Constants::MODULE_IDENTIFIER),
                    Constants::TAB_TITLE, QIcon(Constants::SENDING_ICON_PATH)),
      m_model(std::make_unique<SendingModel>()),
      m_view(std::make_unique<SendingView>()),
      m_delegate(new SendingDelegate(this)),
      m_sendingWorker(std::make_unique<RepeatedSendingWorker>())
{
    m_view->setModel(m_model.get());

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component constructed");
}

SendingComponent::~SendingComponent()
{
    stopRepeatedSending();
    m_parseErrorConn.release();
    m_parseSuccessConn.release();
    if (m_view && m_view->parent())
    {
        m_view.release();
    }
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component destroyed");
}

void SendingComponent::onStart()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Starting Sending Component...");

    m_startTime = std::chrono::steady_clock::now();

    setupConnections();
    setupBrokerSubscriptions();

    checkDeviceReadiness();

    m_eventBroker.publish<Core::ModuleStartedEvent>(
        Core::ModuleStartedEvent(std::type_index(typeid(*this))));
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component started");
}

void SendingComponent::onStop()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Stopping Sending Component...");

    // Stop any active cyclic transmissions
    stopRepeatedSending();

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component stopped");
}

auto SendingComponent::getView() -> QWidget*
{
    return m_view.get();
}

void SendingComponent::setVariableRegistry(Math::VariableRegistry* registry)
{
    m_variableRegistry = registry;
}

void SendingComponent::onDbcConfigReceived(const Core::DbcConfig& config)
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Processing DBC config on UI thread");

    m_model->updateDbcConfig(config);
    if (m_variableRegistry)
    {
        m_view->dbcSubView()->populateFromModel(m_model.get(), *m_variableRegistry);
    }

    LOG_INF(Constants::MODULE_IDENTIFIER, "Created {} message cards",
            config.messageDefinitions.size());
    emit dbcConfigurationChanged(config);
}

void SendingComponent::onDbcParseError() const
{
    m_model->clearEvaluators();
    m_view->dbcSubView()->clearMessages();
}

void SendingComponent::publishRawMessageAsync(const Core::RawCanMessage& message)
{
    QPointer<SendingComponent> self = this;
    (void)QtConcurrent::run([self, message]() {
        if (!self)
        {
            return;
        }
        // Subscribers must ensure thread-safety and proper thread affinity.
        std::scoped_lock lock(self->m_brokerMutex);
        self->m_eventBroker.publish(Core::SendCanMessageRawEvent(message));
    });
}

void SendingComponent::publishDbcMessageAsync(const Core::DbcCanMessage& message)
{
    QPointer<SendingComponent> self = this;
    (void)QtConcurrent::run([self, message]() -> void {
        if (!self)
        {
            return;  // Component was destroyed
        }

        // Subscribers must ensure thread-safety and proper thread affinity.
        std::scoped_lock lock(self->m_brokerMutex);
        self->m_eventBroker.publish(Core::SendCanMessageDbcEvent(message));
    });
}

void SendingComponent::setupConnections()
{
    // Model requests to send raw messages - single path from Model to event broker
    connect(m_model.get(), &SendingModel::requestSendRaw, this,
            [this](const std::string&, const Core::RawCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "Raw send requested: ID=0x{:03X}, DLC={}",
                        message.messageId, message.dlc);
                publishRawMessageAsync(message);
            });

    // Model requests to send DBC messages - single path from Model to event broker
    connect(m_model.get(), &SendingModel::requestSendDbc, this,
            [this](const std::string&, const Core::DbcCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "DBC send requested: ID=0x{:03X}",
                        message.messageId);
                publishDbcMessageAsync(message);
            });

    // View requests: single send
    connect(m_view.get(), &SendingView::sendOnceRequested, this, &SendingComponent::sendOnce);

    connect(m_view.get(), &SendingView::startRepeatedSendingRequested, this,
            &SendingComponent::startRepeatedSending);
    connect(m_view.get(), &SendingView::stopRepeatedSendingRequested, this,
            &SendingComponent::stopRepeatedSending);

    // Worker error handling
    connect(
        m_sendingWorker.get(), &RepeatedSendingWorker::errorOccurred, this,
        [this](const QString& error) {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "Repeated sending error: {}",
                    error.toStdString());
            stopRepeatedSending();

            const auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - m_startTime);

            m_eventBroker.publish<Core::ModuleStoppedEvent>(Core::ModuleStoppedEvent(
                std::type_index(typeid(*this)),
                Core::ModuleDiagnostics{
                    .runtime = runtime, .wasError = true, .errorMessage = error.toStdString()}));
        },
        Qt::QueuedConnection);
}

void SendingComponent::startRepeatedSending(const int intervalMs) const
{
    if (m_sendingWorker->isSending())
    {
        return;
    }

    LOG_INF(Constants::MODULE_IDENTIFIER, "Starting repeated sending, interval={}ms", intervalMs);

    auto callback = [this]() {
        // Called from worker thread - build messages and publish directly to broker
        m_model->forEachPendingMessage(
            [this](const Core::RawCanMessage& message) {
                std::scoped_lock lock(m_brokerMutex);
                m_eventBroker.publish(Core::SendCanMessageRawEvent(message));
            },
            [this](const Core::DbcCanMessage& message) {
                std::scoped_lock lock(m_brokerMutex);
                m_eventBroker.publish(Core::SendCanMessageDbcEvent(message));
            });
    };

    m_sendingWorker->startSending(callback, intervalMs);
    m_model->setTransmissionStatus(true);
}

void SendingComponent::stopRepeatedSending() const
{
    if (!m_sendingWorker->isSending())
    {
        m_model->setTransmissionStatus(false);
        return;
    }

    LOG_INF(Constants::MODULE_IDENTIFIER, "Stopping repeated sending");
    m_sendingWorker->stopSending();
    m_model->setTransmissionStatus(false);
}

void SendingComponent::sendOnce() const
{
    m_model->transmitCurrent();
}

void SendingComponent::setupBrokerSubscriptions()
{
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            LOG_INF(Constants::MODULE_IDENTIFIER, "DBC parse succeeded, queuing to UI thread");
            Core::DbcConfig configCopy = event.config;
            QMetaObject::invokeMethod(
                this, [this, configCopy]() { onDbcConfigReceived(configCopy); },
                Qt::QueuedConnection);
        });

    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "DBC parse failed: {}", event.errorMessage);

            QMetaObject::invokeMethod(this, &SendingComponent::onDbcParseError,
                                      Qt::QueuedConnection);
        });

    m_canDriverChangeConn = m_eventBroker.subscribe<Core::CanDriverChangeEvent>(
        [this](const Core::CanDriverChangeEvent&) {
            QMetaObject::invokeMethod(
                this, [this]() { checkDeviceReadiness(); }, Qt::QueuedConnection);
        });
}

void SendingComponent::checkDeviceReadiness() const
{
    bool isReady = false;
    m_eventBroker.publish<Core::CheckCanDeviceReadyEvent>(Core::CheckCanDeviceReadyEvent(isReady));

    if (isReady == m_lastDeviceReadyState)
    {
        return;
    }

    m_lastDeviceReadyState = isReady;

    if (isReady)
    {
        m_view->hideDeviceNotConfiguredOverlay();
        LOG_INF(Constants::MODULE_IDENTIFIER, "CAN device is ready");
    } else
    {
        m_view->showDeviceNotConfiguredOverlay();
        LOG_WRN(Constants::MODULE_IDENTIFIER, "CAN device not configured");
    }
}

}  // namespace Sending
