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

#include "dbc_component.hpp"

#include "constants.hpp"
#include "util/util.hpp"

namespace DbcFile {

DbcComponent::~DbcComponent()
{
    if (m_view && m_view->parent())
    {
        m_view.release();
    }
}

DbcComponent::DbcComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, Constants::Component::TabId, Constants::Component::TabTitle,
                          QIcon(Constants::Component::TabIcon))
{
    m_model = std::make_unique<DbcModel>(this);
    m_view = std::make_unique<DbcView>();
    m_view->setSourceModel(m_model.get());
}
auto DbcComponent::getView() -> QWidget*
{
    return m_view.get();
}

auto DbcComponent::getModel() const -> DbcModel*
{
    return m_model.get();
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
    m_eventBroker.publish(event);
}
void DbcComponent::onDbcParsed(const Core::DBCParsedEvent& event)
{
    m_model->setDbcConfig(event.config);

    m_view->getLoadPage().showStatusMessage(Constants::Status::ParseSuccess, false);
    m_view->setNavigationEnabled(true);

    // Units for signals page filtering
    const QStringList units = Util::extractSignalUnits(event.config);
    m_view->setSignalUnits(units);

    // Senders for messages page filtering
    const QStringList senders = Util::extractSenders(event.config);
    m_view->setAvailableSenders(senders);
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
