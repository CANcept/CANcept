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
    dbcParsingThread = std::thread([this]() {
        std::unique_lock lock(dbcParsingMutex);
        while (true)
        {
            dbcParsingCondition.wait(lock);
            if (stop_thread) [[unlikely]]
            {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::string* parsedFile = fileParser.parseFile(parseDBCRequestEvent.filePath);
            if (parsedFile == nullptr)
            {
                m_eventBroker.publish(Core::DBCParseErrorEvent("File could not be parsed to string",
                                                               parseDBCRequestEvent.filePath));
                return;
            }
            dbcParser.provideNewFile(*parsedFile);
            delete parsedFile;
            currentDbc = std::move(dbcParser.parseDbc());
            if (currentDbc.get() == nullptr)
            {
                m_eventBroker.publish(Core::DBCParseErrorEvent(
                    "File could not be parsed to DbcConfig", parseDBCRequestEvent.filePath));
                return;
            }
            currentDbc->metaData.fileName = parseDBCRequestEvent.filePath.substr(
                parseDBCRequestEvent.filePath.find_last_of('/') + 1);
            m_eventBroker.publish(
                Core::DBCParsedEvent(*currentDbc.get(), parseDBCRequestEvent.filePath));
        }
    });
    parseNewDbcConnection = m_eventBroker.subscribe<Core::ParseDBCRequestEvent>(
        [this](const Core::ParseDBCRequestEvent& event) -> void { parseNewDbc(event); });
}
void DbcHandler::onStop()
{
    parseNewDbcConnection.release();
    stop_thread = true;
    {
        std::unique_lock lock(dbcParsingMutex);
    }
    dbcParsingCondition.notify_one();
    if (dbcParsingThread.joinable())
    {
        dbcParsingThread.join();
    }
}
DbcHandler::~DbcHandler()
{
    onStop();
}
void DbcHandler::parseNewDbc(const Core::ParseDBCRequestEvent& event)
{
    {
        std::unique_lock lock(dbcParsingMutex);
        parseDBCRequestEvent = event;
    }
    dbcParsingCondition.notify_one();
}

}  // namespace CanHandler