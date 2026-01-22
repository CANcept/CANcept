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

    connect(m_view.get(), &DbcView::fileLoadRequested, this,
            [](const QString& path) { qDebug() << "File Selected: " << path; });
}
DbcComponent::~DbcComponent() = default;
auto DbcComponent::getView() -> QWidget*
{
    return m_view.get();
}
void DbcComponent::onStart() {}
void DbcComponent::onStop() {}
void DbcComponent::onFileLoadRequested(const QString& filePath) const
{
    Core::ParseDBCRequestEvent event;
    event.filePath = filePath.toStdString();
    m_eventBroker.publish(event);
}
void DbcComponent::onDbcParsed(const Core::DBCParsedEvent& event) const
{
    m_view->getLoadPage().showStatusMessage(Constants::Status::Success, false);
    m_view->setNavigationEnabled(true);
}
void DbcComponent::onDbcParseError(const Core::DBCParseErrorEvent& event) const
{
    QString errorMsg = Constants::Status::ErrorPrefix + QString::fromStdString(event.errorMessage);
    m_view->getLoadPage().showStatusMessage(errorMsg, true);
}
void DbcComponent::setupConnections() {}

}  // namespace DbcFile
