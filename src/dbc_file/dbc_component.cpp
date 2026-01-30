//
// Created by jemand on 31.12.25.
//
#include "dbc_component.hpp"

#include "constants.hpp"

namespace DbcFile {

DbcComponent::DbcComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::Component::TabId, Constants::Component::TabTitle,
                          QIcon(Constants::Component::TabIcon))
{
    m_view = std::make_unique<DbcView>();
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
void DbcComponent::onFileLoadRequested(const QString& filePath)
{
    Core::ParseDBCRequestEvent event(filePath.toStdString());
    event.filePath = filePath.toStdString();
    m_eventBroker.publish(event);
}
void DbcComponent::onDbcParsed(const Core::DBCParsedEvent& event)
{
    m_view->getLoadPage().showStatusMessage(Constants::Status::ParseSuccess, false);
    m_view->setNavigationEnabled(true);
}
void DbcComponent::onDbcParseError(const Core::DBCParseErrorEvent& event)
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
