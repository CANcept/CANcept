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

#pragma once
#include <condition_variable>
#include <thread>

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
    explicit DbcHandler(Core::IEventBroker& eventBroker)
        : ILifecycle(eventBroker), parseDBCRequestEvent(""){};
    ~DbcHandler() override;

   protected:
    void onStart() override;
    void onStop() override;

   private:
    void parseNewDbc(const Core::ParseDBCRequestEvent& event);

    DbcParser dbcParser;
    FileParser fileParser;
    Core::Connection parseNewDbcConnection;
    std::unique_ptr<Core::DbcConfig> currentDbc;
    std::condition_variable dbcParsingCondition;
    std::atomic_bool stop_thread = false;
    std::mutex dbcParsingMutex;
    Core::ParseDBCRequestEvent parseDBCRequestEvent;
    std::thread dbcParsingThread;
};
}  // namespace CanHandler
