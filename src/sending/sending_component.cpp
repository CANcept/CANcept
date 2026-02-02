#include "sending_component.hpp"

#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>

#include "constants.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/macro/console_logging.hpp"

namespace Sending {

SendingComponent::SendingComponent(Core::IEventBroker& broker)
    : ITabComponent(broker, QString::fromStdString(Constants::MODULE_IDENTIFIER),
                    Constants::TAB_TITLE, QIcon(Constants::SENDING_ICON_PATH)),
      m_model(std::make_unique<SendingModel>()),
      m_view(std::make_unique<SendingView>()),
      m_delegate(new SendingDelegate(this))
{
    m_view->setModel(m_model.get());

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component constructed");
}

SendingComponent::~SendingComponent()
{
    m_parseErrorConn.release();
    m_parseSuccessConn.release();
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component destroyed");
}

void SendingComponent::onStart()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Starting Sending Component...");

    setupConnections();
    setupBrokerSubscriptions();

    // Populate available CAN devices/interfaces from constants
    m_view->setAvailableDevices(Constants::DEFAULT_CAN_INTERFACES);

    m_eventBroker.publish<Core::ModuleStartedEvent>(
        Core::ModuleStartedEvent(std::type_index(typeid(*this))));
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component started");
}

void SendingComponent::onStop()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Stopping Sending Component...");

    // Stop any active cyclic transmissions
    m_model->setTransmissionStatus(false);

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component stopped");
}

auto SendingComponent::getView() -> QWidget*
{
    return m_view.get();
}

void SendingComponent::onDbcConfigReceived(const Core::DbcConfig& config)
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Processing DBC config on UI thread");

    m_model->updateDbcConfig(config);
    m_view->dbcSubView()->populateFromModel(m_model.get());

    LOG_INF(Constants::MODULE_IDENTIFIER, "Created {} message cards",
            config.messageDefinitions.size());
    emit dbcConfigurationChanged(config);
}

void SendingComponent::onDbcParseError() const
{
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
    // Device selection changes
    connect(m_view.get(), &SendingView::deviceSelectionChanged, this,
            [this](const std::string& deviceName) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "CAN device changed to: {}", deviceName);
                m_eventBroker.publish<Core::CanDriverChangeEvent>(
                    Core::CanDriverChangeEvent(deviceName));
            });

    // Raw send requested from view - defer to worker thread
    connect(m_view.get(), &SendingView::sendRawRequested, this,
            [this](const Core::RawCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "Raw send requested: ID=0x{:03X}, DLC={}",
                        message.messageId, message.dlc);
                publishRawMessageAsync(message);
            });

    // DBC send requested from view - defer to worker thread
    connect(m_view.get(), &SendingView::sendDbcRequested, this,
            [this](const Core::DbcCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "DBC send requested: ID=0x{:03X}",
                        message.messageId);
                publishDbcMessageAsync(message);
            });

    // Model requests to send raw messages defer to worker thread
    connect(m_model.get(), &SendingModel::requestSendRaw, this,
            [this](const std::string&, const Core::RawCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "Model cyclic raw transmission: ID=0x{:03X}",
                        message.messageId);
                publishRawMessageAsync(message);
            });

    // Model requests to send DBC messages defer to worker thread
    connect(m_model.get(), &SendingModel::requestSendDbc, this,
            [this](const std::string&, const Core::DbcCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "Model cyclic DBC transmission: ID=0x{:03X}",
                        message.messageId);
                publishDbcMessageAsync(message);
            });
}

void SendingComponent::setupBrokerSubscriptions()
{
    // Subscribe to DBC parsing success events
    // Use Qt::QueuedConnection to defer UI updates to the main thread
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            LOG_INF(Constants::MODULE_IDENTIFIER, "DBC parse succeeded, queuing to UI thread");
            Core::DbcConfig configCopy = event.config;
            QMetaObject::invokeMethod(
                this, [this, configCopy]() { onDbcConfigReceived(configCopy); },
                Qt::QueuedConnection);
        });

    // Subscribe to DBC parse error events
    // Use Qt::QueuedConnection to defer UI updates to the main thread
    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "DBC parse failed: {}", event.errorMessage);

            QMetaObject::invokeMethod(this, &SendingComponent::onDbcParseError,
                                      Qt::QueuedConnection);
        });
}

}  // namespace Sending
