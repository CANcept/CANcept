//
// Created by flori on 30.12.2025.
//

#ifndef CANBUSMANAGER_DBC_HANDLER_HPP
#define CANBUSMANAGER_DBC_HANDLER_HPP
#include "core/event/dbc_event.hpp"
#include "core/interface/i_lifecycle.hpp"
#include "dbc_parser.hpp"
#include "file_parser.hpp"
namespace CanHandler {
/**
 * @brief The DbcHandler is responsible for parsing DBC configurations from a file.
 */
class DbcHandler final : public Core::ILifecycle
{
   public:
    explicit DbcHandler(Core::IEventBroker& eventBroker) : Core::ILifecycle(eventBroker)
    {
        parseNewDbcConnection = eventBroker.subscribe<Core::ParseDBCRequestEvent>(
            [this](const Core::ParseDBCRequestEvent& event) -> void { parseNewDbc(event); });
    };
    ~DbcHandler() override;

   protected:
    void onStart() override;
    void onStop() override;

   private:
    void parseNewDbc(const Core::ParseDBCRequestEvent& event);

    DbcParser dbcParser;
    FileParser fileParser;
    Core::Connection parseNewDbcConnection;
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_DBC_HANDLER_HPP
