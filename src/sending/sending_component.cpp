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
    // DBC View: Message selection changes
    connect(m_view->dbcSubView(), &DbcSendingSubView::messageSelectionChanged, this,
            [this](uint32_t messageId, bool selected) {
                LOG_INF("SendingComponent", "Message 0x{:X} selection changed: {}", messageId,
                        selected);
                m_model->setMessageSelected(messageId, selected);
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
            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
                const auto deviceName =
                    m_view->rawSubView()->interfaceSelector()->currentText().toStdString();
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
        message.receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        // Parse CAN ID (hex format, may have "0x" prefix and spaces)
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

            LOG_INF("SendingComponent", "Created {} message cards",
                    event.config.messageDefinitions.size());
            emit dbcConfigurationChanged(event.config);
        });

    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) {
            LOG_ERR("SendingComponent", "DBC parse failed: {}", event.errorMessage);
            m_view->dbcSubView()->clearMessages();
        });
}

void SendingComponent::onDeviceChanged(const std::string& deviceName)
{
    LOG_INF("SendingComponent", "CAN device changed to: {}", deviceName);
    m_eventBroker.publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent(deviceName));
}

void SendingComponent::onSendRawRequested(const Core::RawCanMessage& message)
{
    LOG_INF("SendingComponent", "Publishing raw CAN message: ID=0x{:03X}", message.messageId);
    m_eventBroker.publish(Core::SendCanMessageRawEvent(message));
}

void SendingComponent::onSendDbcRequested(const Core::DbcCanMessage& message)
{
    LOG_INF("SendingComponent", "Publishing DBC CAN message");
    m_eventBroker.publish(Core::SendCanMessageDbcEvent(message));
}

void SendingComponent::loadTestDbcData()
{
    LOG_INF("SendingComponent", "Loading temporary test DBC data");

    // Create test DBC configuration
    static Core::DbcConfig testConfig;

    // Clear previous data
    testConfig.nodeDefinitions.clear();
    testConfig.messageDefinitions.clear();
    testConfig.signalValueDescriptions.clear();
    testConfig.comments.clear();

    // Add test nodes
    testConfig.nodeDefinitions.push_back("ECU");
    testConfig.nodeDefinitions.push_back("Motor");
    testConfig.nodeDefinitions.push_back("Dashboard");

    // Message 1: Engine Data (CAN ID 0x100)
    Core::DbcMessageDescription engineMsg;
    engineMsg.messageId = 0x100;
    engineMsg.messageName = "EngineData";
    engineMsg.messageSize = 8;
    engineMsg.transmitterName = "ECU";

    Core::DbcSignalDescription rpmSignal;
    rpmSignal.signalName = "EngineRPM";
    rpmSignal.multiplexer = false;
    rpmSignal.multiplexedBy = -1;
    rpmSignal.startBit = 0;
    rpmSignal.signalSize = 16;
    rpmSignal.byteOrder = false;
    rpmSignal.valueType = false;
    rpmSignal.factor = 1.0;
    rpmSignal.offset = 0.0;
    rpmSignal.minimum = 0.0;
    rpmSignal.maximum = 8000.0;
    rpmSignal.unit = "rpm";
    rpmSignal.receivers.push_back("Dashboard");

    Core::DbcSignalDescription tempSignal;
    tempSignal.signalName = "EngineTemp";
    tempSignal.multiplexer = false;
    tempSignal.multiplexedBy = -1;
    tempSignal.startBit = 16;
    tempSignal.signalSize = 8;
    tempSignal.byteOrder = false;
    tempSignal.valueType = false;
    tempSignal.factor = 1.0;
    tempSignal.offset = -40.0;
    tempSignal.minimum = -40.0;
    tempSignal.maximum = 215.0;
    tempSignal.unit = "degC";
    tempSignal.receivers.push_back("Dashboard");

    engineMsg.signalDescriptions.push_back(rpmSignal);
    engineMsg.signalDescriptions.push_back(tempSignal);

    // Message 2: Vehicle Speed (CAN ID 0x200)
    Core::DbcMessageDescription speedMsg;
    speedMsg.messageId = 0x200;
    speedMsg.messageName = "VehicleSpeed";
    speedMsg.messageSize = 8;
    speedMsg.transmitterName = "ECU";

    Core::DbcSignalDescription speedSignal;
    speedSignal.signalName = "Speed";
    speedSignal.multiplexer = false;
    speedSignal.multiplexedBy = -1;
    speedSignal.startBit = 0;
    speedSignal.signalSize = 16;
    speedSignal.byteOrder = false;
    speedSignal.valueType = false;
    speedSignal.factor = 0.01;
    speedSignal.offset = 0.0;
    speedSignal.minimum = 0.0;
    speedSignal.maximum = 250.0;
    speedSignal.unit = "km/h";
    speedSignal.receivers.push_back("Dashboard");

    speedMsg.signalDescriptions.push_back(speedSignal);

    // Message 3: Brake Status (CAN ID 0x300)
    Core::DbcMessageDescription brakeMsg;
    brakeMsg.messageId = 0x300;
    brakeMsg.messageName = "BrakeStatus";
    brakeMsg.messageSize = 4;
    brakeMsg.transmitterName = "Motor";

    Core::DbcSignalDescription brakePressureSignal;
    brakePressureSignal.signalName = "BrakePressure";
    brakePressureSignal.multiplexer = false;
    brakePressureSignal.multiplexedBy = -1;
    brakePressureSignal.startBit = 0;
    brakePressureSignal.signalSize = 16;
    brakePressureSignal.byteOrder = false;
    brakePressureSignal.valueType = false;
    brakePressureSignal.factor = 0.1;
    brakePressureSignal.offset = 0.0;
    brakePressureSignal.minimum = 0.0;
    brakePressureSignal.maximum = 200.0;
    brakePressureSignal.unit = "bar";
    brakePressureSignal.receivers.push_back("ECU");

    Core::DbcSignalDescription brakeActiveSignal;
    brakeActiveSignal.signalName = "BrakeActive";
    brakeActiveSignal.multiplexer = false;
    brakeActiveSignal.multiplexedBy = -1;
    brakeActiveSignal.startBit = 16;
    brakeActiveSignal.signalSize = 1;
    brakeActiveSignal.byteOrder = false;
    brakeActiveSignal.valueType = false;
    brakeActiveSignal.factor = 1.0;
    brakeActiveSignal.offset = 0.0;
    brakeActiveSignal.minimum = 0.0;
    brakeActiveSignal.maximum = 1.0;
    brakeActiveSignal.unit = "";
    brakeActiveSignal.receivers.push_back("ECU");

    brakeMsg.signalDescriptions.push_back(brakePressureSignal);
    brakeMsg.signalDescriptions.push_back(brakeActiveSignal);

    // Add messages to config
    testConfig.messageDefinitions.push_back(engineMsg);
    testConfig.messageDefinitions.push_back(speedMsg);
    testConfig.messageDefinitions.push_back(brakeMsg);

    // Set metadata
    testConfig.metaData.version = "1.0";
    testConfig.metaData.fileName = "test_data.dbc";

    // Update model (Model owns data)
    m_model->updateDbcConfig(testConfig);

    // View populates itself from model (strict MVD)
    m_view->dbcSubView()->populateFromModel(m_model.get());

    LOG_INF("SendingComponent", "Test DBC data loaded successfully");
}

}  // namespace Sending