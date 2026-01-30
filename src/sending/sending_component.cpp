#include "sending_component.hpp"

#include <QRegularExpression>
#include <ctime>

#include "constants.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/macro/console_logging.hpp"
#include "view/dbc_based_sending_subview.hpp"

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
    m_parseErrorConn.release();
    m_parseSuccessConn.release();
    LOG_INF("SendingComponent", "Sending Component destroyed");
}

void SendingComponent::onStart()
{
    LOG_INF("SendingComponent", "Starting Sending Component...");

    setupConnections();
    setupBrokerSubscriptions();

    // Populate available CAN devices/interfaces from constants
    m_view->setAvailableDevices(Constants::DEFAULT_CAN_INTERFACES);

    // Set available bitrates from constants
    m_view->setAvailableSpeeds(Constants::STANDARD_BIT_RATES);

    // Initialize send buttons as disabled (no interface/message selected yet)
    updateSendButtonStates();

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
    // View: Mode changes (Raw vs DBC)
    connect(m_view.get(), &SendingView::modeChanged, this, [this](const bool isDbcMode) {
        const auto mode = isDbcMode ? SendingModel::Mode::Dbc : SendingModel::Mode::Raw;
        m_model->setData(QModelIndex(), static_cast<int>(mode), SendingModel::Role_ActiveMode);
        LOG_INF("SendingComponent", "Mode changed to: {}", isDbcMode ? "DBC" : "Raw");
    });

    // DBC View: Message selection changes
    connect(m_view->dbcSubView(), &DbcSendingSubView::messageSelectionChanged, this,
            [this](uint32_t messageId, bool selected) {
                LOG_INF("SendingComponent", "Message 0x{:X} selection changed: {}", messageId,
                        selected);
                m_model->setMessageSelected(messageId, selected);
                updateSendButtonStates();
            });

    // DBC View: Signal selection changes
    connect(m_view->dbcSubView(), &DbcSendingSubView::signalSelectionChanged, this,
            [this](const QString& signalName, bool selected) {
                LOG_INF("SendingComponent", "Signal {} selection changed: {}",
                        signalName.toStdString(), selected);
                m_model->setSignalSelected(signalName.toStdString(), selected);
                updateSendButtonStates();
            });

    // DBC View: Signal value changes
    connect(m_view->dbcSubView(), &DbcSendingSubView::signalValueChanged, this,
            [this](const QString& signalName, double newValue) {
                LOG_INF("SendingComponent", "Signal {} value changed to {}",
                        signalName.toStdString(), newValue);
                const Core::DbcConfig* dbc = m_model->currentDbcConfig();
                if (!dbc)
                {
                    return;
                }

                // Find message and signal indices
                int msgRow = 0;
                for (const auto& msg : dbc->messageDefinitions)
                {
                    int sigRow = 0;
                    for (const auto& sig : msg.signalDescriptions)
                    {
                        if (sig.signalName == signalName.toStdString())
                        {
                            const QModelIndex msgIndex = m_model->index(msgRow, 0);
                            const QModelIndex sigIndex = m_model->index(sigRow, 0, msgIndex);
                            m_model->setData(sigIndex, newValue, SendingModel::Role_SignalValue);
                            return;
                        }
                        ++sigRow;
                    }
                    ++msgRow;
                }
            });

    // Device selection changes - Raw mode
    connect(m_view->rawSubView()->interfaceSelector(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
                m_rawInterfaceSelected = (index >= 0);
                if (index >= 0)
                {
                    const auto deviceName =
                        m_view->rawSubView()->interfaceSelector()->currentText().toStdString();
                    onDeviceChanged(deviceName);
                }
                updateSendButtonStates();
            });

    // Device selection changes - DBC mode
    connect(m_view->dbcSubView()->interfaceSelector(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
                m_dbcInterfaceSelected = (index >= 0);
                if (index >= 0)
                {
                    const auto deviceName =
                        m_view->dbcSubView()->interfaceSelector()->currentText().toStdString();
                    onDeviceChanged(deviceName);
                }
                updateSendButtonStates();
            });

    // Send button - Raw mode
    connect(m_view->rawSubView()->sendButton(), &QPushButton::clicked, this, [this]() {
        Core::RawCanMessage message{};
        message.receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        // Parse CAN ID
        QString canIdText = m_view->rawSubView()->canIdEditor()->text().trimmed();
        if (canIdText.startsWith("0x", Qt::CaseInsensitive))
        {
            canIdText = canIdText.mid(2);
        }
        canIdText = canIdText.remove(' ');

        bool ok = false;
        const uint32_t canId = canIdText.toUInt(&ok, 16);
        if (!ok || canId > Constants::MAX_CAN_ID)
        {
            LOG_ERR("SendingComponent", "Failed to parse CAN ID from input or ID out of range");
            return;
        }
        message.messageId = static_cast<uint16_t>(canId);
        const QString messageDataText = m_view->rawSubView()->messageDataEditor()->text().trimmed();
        message.data.fill(0);

        if (!messageDataText.isEmpty())
        {
            // Split by whitespace and parse each byte
            QStringList byteStrings =
                messageDataText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

            int dlc = 0;
            for (int i = 0; i < byteStrings.size() && i < static_cast<int>(Constants::MAX_CAN_DLC);
                 ++i)
            {
                const uint8_t byte = byteStrings[i].toUInt(&ok, 16);
                if (!ok)
                {
                    LOG_ERR("SendingComponent", "Failed to parse data byte {} from input: '{}'", i,
                            byteStrings[i].toStdString());
                    return;
                }
                message.data[static_cast<size_t>(i)] = static_cast<char>(byte);
                dlc++;
            }

            message.dlc = static_cast<uint8_t>(dlc);
            LOG_INF("SendingComponent", "Raw send button clicked: ID=0x{:03X}, DLC={}",
                    message.messageId, message.dlc);
        } else
        {
            message.dlc = 0;
            LOG_INF("SendingComponent", "Raw send button clicked: ID=0x{:03X}, DLC=0 (no data)",
                    message.messageId);
        }

        onSendRawRequested(message);
    });

    // Send button - DBC mode
    connect(m_view->dbcSubView()->sendButton(), &QPushButton::clicked, this, [this]() {
        LOG_INF("SendingComponent", "DBC send button clicked, triggering model transmission");
        m_model->transmitCurrent();
    });

    // Model requests to send raw messages
    connect(m_model.get(), &SendingModel::requestSendRaw, this,
            [this](const std::string& device, const Core::RawCanMessage& message) {
                LOG_INF("SendingComponent",
                        "Model requests raw transmission: device={}, ID=0x{:02X}", device,
                        static_cast<uint8_t>(message.messageId));
                onSendRawRequested(message);
            });

    // Model requests to send DBC messages
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
            m_view->dbcSubView()->populateFromModel(m_model.get());
            updateSendButtonStates();

            LOG_INF("SendingComponent", "Created {} message cards",
                    event.config.messageDefinitions.size());
            emit dbcConfigurationChanged(event.config);
        });

    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) {
            LOG_ERR("SendingComponent", "DBC parse failed: {}", event.errorMessage);
            m_view->dbcSubView()->clearMessages();
            updateSendButtonStates();
        });
}

void SendingComponent::onDeviceChanged(const std::string& deviceName) const
{
    LOG_INF("SendingComponent", "CAN device changed to: {}", deviceName);
    m_eventBroker.publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent(deviceName));
}

void SendingComponent::onSendRawRequested(const Core::RawCanMessage& message) const
{
    LOG_INF("SendingComponent", "Publishing raw CAN message: ID=0x{:03X}", message.messageId);
    m_eventBroker.publish(Core::SendCanMessageRawEvent(message));
}

void SendingComponent::onSendDbcRequested(const Core::DbcCanMessage& message) const
{
    LOG_INF("SendingComponent", "Publishing DBC CAN message");
    m_eventBroker.publish(Core::SendCanMessageDbcEvent(message));
}

void SendingComponent::updateSendButtonStates() const
{
    if (auto* rawSendBtn = m_view->rawSubView()->sendButton())
    {
        rawSendBtn->setEnabled(m_rawInterfaceSelected);
    }

    if (auto* dbcSendBtn = m_view->dbcSubView()->sendButton())
    {
        bool anySignalSelected = false;
        if (m_model->currentDbcConfig())
        {
            for (const auto& msg : m_model->currentDbcConfig()->messageDefinitions)
            {
                for (const auto& sig : msg.signalDescriptions)
                {
                    if (m_model->isSignalSelected(sig.signalName))
                    {
                        anySignalSelected = true;
                        break;
                    }
                }
                if (anySignalSelected)
                {
                    break;
                }
            }
        }

        dbcSendBtn->setEnabled(m_dbcInterfaceSelected && anySignalSelected);
    }
}

}  // namespace Sending