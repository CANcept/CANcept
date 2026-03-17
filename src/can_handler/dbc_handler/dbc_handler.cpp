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

#include "dbc_handler.hpp"

#include <algorithm>

namespace CanHandler {
void DbcHandler::onStart()
{
    parseNewDbcConnection = m_eventBroker.subscribe<Core::ParseDBCRequestEvent>(
        [this](const Core::ParseDBCRequestEvent& event) -> void { parseNewDbc(event); });
}
void DbcHandler::onStop()
{
    parseNewDbcConnection.release();
}
DbcHandler::~DbcHandler()
{
    onStop();
}
void DbcHandler::parseNewDbc(const Core::ParseDBCRequestEvent& event)
{
    std::string* parsedFile = fileParser.parseFile(event.filePath);
    if (parsedFile == nullptr)
    {
        m_eventBroker.publish(
            Core::DBCParseErrorEvent("File could not be parsed to string", event.filePath));
        return;
    }
    dbcParser.provideNewFile(*parsedFile);
    delete parsedFile;
    currentDbc = std::move(dbcParser.parseDbc());
    if (currentDbc.get() == nullptr)
    {
        m_eventBroker.publish(
            Core::DBCParseErrorEvent("File could not be parsed to DbcConfig", event.filePath));
        return;
    }
    m_eventBroker.publish(Core::DBCParsedEvent(*currentDbc.get(), event.filePath));
}

}  // namespace CanHandler