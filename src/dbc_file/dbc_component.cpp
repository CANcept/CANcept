#include "dbc_component.hpp"

#include "constants.hpp"

namespace DbcFile {
DbcComponent::DbcComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::Component::TabId, Constants::Component::TabTitle,
                          QIcon(Constants::Component::TabIcon))
{
    m_model = std::make_unique<DbcModel>(broker, this);
    m_view = std::make_unique<DbcView>();
    m_view->setSourceModel(m_model.get());
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

auto DbcComponent::extractSignalUnits(const Core::DBCParsedEvent& event) -> QStringList
{
    QSet<QString> uniqueUnits;

    for (const auto& msg : event.config.messageDefinitions) {
        for (const auto& sig : msg.signalDescriptions) {
            if (!sig.unit.empty()) {
                uniqueUnits.insert(QString::fromStdString(sig.unit));
            }
        }
    }

    QStringList sortedUnits = uniqueUnits.values();
    sortedUnits.sort(Qt::CaseInsensitive);
    return sortedUnits;
}
void DbcComponent::onFileLoadRequested(const QString& filePath) const
{
    Core::ParseDBCRequestEvent event(filePath.toStdString());
    m_eventBroker.publish(event);
}
void DbcComponent::onDbcParsed(const Core::DBCParsedEvent& event) const
{
    m_view->getLoadPage().showStatusMessage(Constants::Status::ParseSuccess, false);
    m_view->setNavigationEnabled(true);

    const QStringList units = extractSignalUnits(event);
    m_view->setSignalUnits(units);
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
