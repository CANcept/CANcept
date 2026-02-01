//
// Created by jemand on 31.12.25.
//
#include "dbc_component.hpp"

#include "constants.hpp"

namespace DbcFile {

void DbcComponent::runUiTest() {
    // 1. Fake DTO bauen
    Core::DbcConfig config;

    // 1. Metadata
    config.metaData.fileName = "Demo_Vehicle.dbc";
    config.metaData.version = "v2.3 (Test)";

    // 2. ECUs (Nodes)
    // Wir definieren 2 normale ECUs
    config.nodeDefinitions.push_back("EngineECU");
    config.nodeDefinitions.push_back("DashboardECU");

    // 3. Messages & Signals

    // --- Message 1: RPM (Von EngineECU) ---
    Core::DbcMessageDescription msg1;
    msg1.messageName = "EngineData";
    msg1.messageId = 100;
    msg1.messageSize = 8;
    msg1.transmitterName = "EngineECU"; // Matcht mit Node oben

    // Signal: RPM
    Core::DbcSignalDescription sig1;
    sig1.signalName = "EngineRPM";
    sig1.startBit = 0;
    sig1.signalSize = 16;
    sig1.factor = 1.0;
    sig1.offset = 0.0;
    sig1.minimum = 0;
    sig1.maximum = 8000;
    sig1.unit = "rpm";
    sig1.byteOrder = false; // Little Endian
    sig1.valueType = false; // Unsigned
    sig1.receivers.push_back("DashboardECU");

    msg1.signalDescriptions.push_back(sig1);
    config.messageDefinitions.push_back(msg1);

    // --- Message 2: Speed (Von EngineECU) ---
    Core::DbcMessageDescription msg2;
    msg2.messageName = "VehicleSpeed";
    msg2.messageId = 200;
    msg2.messageSize = 8;
    msg2.transmitterName = "EngineECU";

    Core::DbcSignalDescription sig2;
    sig2.signalName = "Speed";
    sig2.startBit = 0;
    sig2.signalSize = 16;
    sig2.factor = 0.01;
    sig2.unit = "km/h";

    msg2.signalDescriptions.push_back(sig2);
    config.messageDefinitions.push_back(msg2);

    // --- Message 3: Orphan (Kein bekannter Sender) ---
    Core::DbcMessageDescription msg3;
    msg3.messageName = "DebugMessage";
    msg3.messageId = 999;
    msg3.messageSize = 4;
    msg3.transmitterName = "UnknownSender"; // Existiert nicht in nodeDefinitions!
    // -> Sollte als Orphan gezählt werden

    config.messageDefinitions.push_back(msg3);

    // 2. Event simulieren
    // Wir rufen den Handler direkt auf (ohne Broker)
    Core::DBCParsedEvent event(config, "test");

    // Zugriff auf private Methode 'onDbcParsed'
    // Da wir IN der Klasse sind, geht das!
    m_model->onDbcParsed(event);
    this->onDbcParsed(event);

    qDebug() << "UI Test Data injected!";
}

DbcComponent::DbcComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::Component::TabId, Constants::Component::TabTitle,
                          QIcon(Constants::Component::TabIcon))
{
    m_model = std::make_unique<DbcModel>(broker, this);
    m_view = std::make_unique<DbcView>();
    m_view->setSourceModel(m_model.get());
    runUiTest();
}
DbcComponent::~DbcComponent() = default;
auto DbcComponent::getView() -> QWidget*
{
    return m_view.get();
}
void DbcComponent::onStart()
{
    setupConnections();
}
void DbcComponent::onStop()
{
    m_parseSuccessConn.release();
    m_parseErrorConn.release();
}
void DbcComponent::onFileLoadRequested(const QString& filePath) const
{
    Core::ParseDBCRequestEvent event(filePath.toStdString());
    event.filePath = filePath.toStdString();
    m_eventBroker.publish(event);
}
void DbcComponent::onDbcParsed(const Core::DBCParsedEvent& event) const
{
    m_view->getLoadPage().showStatusMessage(Constants::Status::ParseSuccess, false);
    m_view->setNavigationEnabled(true);
}
void DbcComponent::onDbcParseError(const Core::DBCParseErrorEvent& event) const
{
    QString errorMsg = Constants::Status::ErrorPrefix + QString::fromStdString(event.errorMessage);
    m_view->getLoadPage().showStatusMessage(errorMsg, true);
    m_view->setNavigationEnabled(false);
}
void DbcComponent::setupConnections()
{
    m_parseSuccessConn = m_eventBroker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) { this->onDbcParsed(event); });

    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) { this->onDbcParseError(event); });

    connect(m_view.get(), &DbcView::fileLoadRequested, this,
            [this](const QString& path) { this->onFileLoadRequested(path); });
}

}  // namespace DbcFile
