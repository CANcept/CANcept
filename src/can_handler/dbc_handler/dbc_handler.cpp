//
// Created by flori on 21.01.2026.
//
#include "dbc_handler.hpp"

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
    const Core::DbcConfig* parsedDbc = dbcParser.parseDbc();
    if (parsedDbc == nullptr)
    {
        m_eventBroker.publish(
            Core::DBCParseErrorEvent("File could not be parsed to DbcConfig", event.filePath));
        return;
    }
    m_eventBroker.publish(Core::DBCParsedEvent(*parsedDbc, event.filePath));
}

}  // namespace CanHandler