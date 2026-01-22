#include "sending_component.hpp"

#include <ctime>

#include "constants.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/macro/console_logging.hpp"
#include "core/widgets/dbc_signal_row.hpp"

namespace Sending {

SendingComponent::SendingComponent(Core::IEventBroker& broker)
    : ITabComponent(broker, QString::fromStdString(Constants::MODULE_IDENTIFIER),
                    Constants::TAB_TITLE, QIcon(Constants::SENDING_ICON_PATH)),
      m_model(std::make_unique<SendingModel>()),
      m_view(std::make_unique<SendingView>()),
      m_delegate(new SendingDelegate(this))
{
    m_view->setModel(m_model.get());

    LOG_INF("SendingComponent", "Sending Component constructed");
}

SendingComponent::~SendingComponent()
{
    LOG_INF("SendingComponent", "Destroying Sending Component...");
}

void SendingComponent::onStart()
{
    LOG_INF("SendingComponent", "Starting Sending Component...");

    setupConnections();
    setupBrokerSubscriptions();

    // Populate available CAN devices/interfaces from constants
    m_view->setAvailableDevices(Constants::DEFAULT_CAN_DEVICES);

    // Set available bitrates from constants
    m_view->setAvailableSpeeds(Constants::STANDARD_BITRATES);

    m_eventBroker.publish<Core::ModuleStartedEvent>(
        Core::ModuleStartedEvent(std::type_index(typeid(*this))));
    LOG_INF("SendingComponent", "Sending Component started");
}

void SendingComponent::onStop()
{
    LOG_INF("SendingComponent", "Stopping Sending Component...");

    // Stop any active cyclic transmissions
    m_model->setTransmissionStatus(false);

    LOG_INF("SendingComponent", "Sending Component stopped");
}

auto SendingComponent::getView() -> QWidget*
{
    return m_view.get();
}

void SendingComponent::setupConnections()
{
    // Device selection changes - Raw mode
    connect(m_view->rawSubView()->deviceSelector(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
                const auto deviceName =
                    m_view->rawSubView()->deviceSelector()->currentText().toStdString();
                onDeviceChanged(deviceName);
            });

    // Device selection changes - DBC mode
    connect(m_view->dbcSubView()->interfaceSelector(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
                const auto deviceName =
                    m_view->dbcSubView()->interfaceSelector()->currentText().toStdString();
                onDeviceChanged(deviceName);
            });

    // Send button - Raw mode
    connect(m_view->rawSubView()->sendButton(), &QPushButton::clicked, this, [this]() {
        Core::RawCanMessage message{};
        message.receiveTime = std::time(nullptr);

        // Parse CAN ID (hex format)
        bool ok = false;
        const uint32_t canId = m_view->rawSubView()->idEditor()->text().toUInt(&ok, 16);
        if (!ok)
        {
            LOG_ERR("SendingComponent", "Failed to parse CAN ID from input");
            return;
        }
        message.messageId = static_cast<char>(canId & 0xFF);

        // Get DLC for determining how many bytes to read
        const int dlc = m_view->rawSubView()->dlcSpinBox()->value();

        // Parse data bytes (hex format) into fixed array
        const auto& byteEditors = m_view->rawSubView()->byteEditors();
        message.data.fill(0);  // Initialize all bytes to 0

        for (int i = 0; i < dlc && i < static_cast<int>(Constants::MAX_CAN_DLC); ++i)
        {
            const uint8_t byte = byteEditors[i]->text().toUInt(&ok, 16);
            if (!ok)
            {
                LOG_ERR("SendingComponent", "Failed to parse data byte {} from input", i);
                return;
            }
            message.data[static_cast<size_t>(i)] = static_cast<char>(byte);
        }

        LOG_INF("SendingComponent", "Raw send button clicked: ID=0x{:02X}, DLC={}",
                static_cast<uint8_t>(message.messageId), dlc);
        onSendRawRequested(message);
    });

    // Send button - DBC mode
    connect(m_view->dbcSubView()->sendButton(), &QPushButton::clicked, this, [this]() {
        LOG_INF("SendingComponent", "DBC send button clicked, triggering model transmission");
        m_model->transmitCurrent();
    });

    // Model requests to send raw messages (from manual trigger or cyclic timer)
    connect(m_model.get(), &SendingModel::requestSendRaw, this,
            [this](const std::string& device, const Core::RawCanMessage& message) {
                LOG_INF("SendingComponent",
                        "Model requests raw transmission: device={}, ID=0x{:02X}", device,
                        static_cast<uint8_t>(message.messageId));
                onSendRawRequested(message);
            });

    // Model requests to send DBC messages (from manual trigger or cyclic timer)
    connect(m_model.get(), &SendingModel::requestSendDbc, this,
            [this](const std::string& device, const Core::DbcCanMessage& message) {
                LOG_INF("SendingComponent", "Model requests DBC transmission: device={}", device);
                onSendDbcRequested(message);
            });
}

void SendingComponent::setupBrokerSubscriptions()
{
    // Subscribe to DBC parsing success events
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            LOG_INF("SendingComponent", "DBC parse succeeded, updating model configuration");
            m_model->updateDbcConfig(event.config);

            // Clear existing message cards
            m_view->dbcSubView()->clearMessages();

            // Create message cards for each DBC message
            for (const auto& msgDef : event.config.messageDefinitions)
            {
                auto* card = new Core::DbcMessageCard(
                    QString::fromStdString(msgDef.messageName), msgDef.messageId,
                    static_cast<int>(msgDef.signalDescriptions.size()), m_view->dbcSubView());

                // Create signal rows for each signal in the message
                for (const auto& sigDef : msgDef.signalDescriptions)
                {
                    auto* signalRow = new Core::DbcSignalRowWidget(
                        QString::fromStdString(sigDef.signalName),
                        QString::fromStdString(sigDef.unit), sigDef.minimum, sigDef.maximum, card);

                    // Connect signal value changes to model
                    connect(signalRow->valueEditor(), &QLineEdit::textChanged, this,
                            [signalName = sigDef.signalName](const QString& text) {
                                bool ok = false;
                                double value = text.toDouble(&ok);
                                if (ok)
                                {
                                    // Find the signal index in the model and update
                                    // For now, store directly in the dynamic values map
                                    // This is a simplified approach
                                    LOG_INF("SendingComponent", "Signal {} value changed to {}",
                                            signalName, value);
                                }
                            });

                    card->addSignalRow(signalRow);
                }

                // Connect card checkbox to track selected messages
                connect(card->headerCheckbox(), &QCheckBox::toggled, this,
                        [msgId = msgDef.messageId](bool checked) {
                            LOG_INF("SendingComponent", "Message 0x{:X} selection changed: {}",
                                    msgId, checked);
                            // Update selected messages in model (will be implemented in US4)
                        });

                m_view->dbcSubView()->addMessageCard(card);
            }

            LOG_INF("SendingComponent", "Created {} message cards",
                    event.config.messageDefinitions.size());
            emit dbcConfigurationChanged(event.config);
        });

    // Subscribe to DBC parsing error events
    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) {
            LOG_ERR("SendingComponent", "DBC parse failed: {}", event.errorMessage);
            // Clear any existing cards and show error state
            m_view->dbcSubView()->clearMessages();
        });
}

void SendingComponent::onDeviceChanged(const std::string& deviceName)
{
    LOG_INF("SendingComponent", "CAN device changed to: {}", deviceName);
    //TODO
}

void SendingComponent::onSendRawRequested(const Core::RawCanMessage& message)
{
    LOG_INF("SendingComponent", "Publishing raw CAN message: ID=0x{:02X}",
            static_cast<uint8_t>(message.messageId));

    // Publish to broker - CAN handler will process this
    Core::SendCanMessageRawEvent event;
    event.canMessage = message;
    m_eventBroker.publish(event);
}

void SendingComponent::onSendDbcRequested(const Core::DbcCanMessage& message)
{
    LOG_INF("SendingComponent", "Publishing DBC CAN message");

    // Publish to broker - CAN handler will process this
    Core::SendCanMessageDbcEvent event;
    event.canMessage = message;
    m_eventBroker.publish(event);
}

}  // namespace Sending