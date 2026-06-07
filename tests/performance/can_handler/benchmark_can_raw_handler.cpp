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

#include <chrono>
#include <memory>

#include "benchmark/benchmark.h"
#include "can_handler/can_communication_handler/can_raw_handler.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "event_broker/event_broker.hpp"
#include "tests/helpers/mock_event_broker.hpp"

class CanRawHandlerBenchmark
{
   public:
    CanRawHandlerBenchmark()
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canRawHandler = std::make_unique<CanHandler::CanRawHandler>(
            *eventBroker, [](const CanMessage&) { return true; });

        eventBroker->publish(Core::AppStartedEvent{});
    }

    ~CanRawHandlerBenchmark()
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        canRawHandler.reset();
        eventBroker.reset();
    }

    std::unique_ptr<Core::IEventBroker> eventBroker;
    std::unique_ptr<CanHandler::CanRawHandler> canRawHandler;
};

static void BM_CanRawHandler_Send_ParseMessage(benchmark::State& state)
{
    CanRawHandlerBenchmark benchmarkObject;
    const Core::RawCanMessage message = {.receiveTime = std::chrono::milliseconds(1000),
                                         .data = {'1', '2', '3', '4', '5', '6', '7', '8'},
                                         .messageId = 0x123,
                                         .dlc = 8};
    const Core::SendCanMessageRawEvent event(message);

    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanRawHandler_Send_ParseMessage);

static void BM_CanRawHandler_Receive_ParseMessage_FullFrame(benchmark::State& state)
{
    CanRawHandlerBenchmark benchmarkObject;
    const CanMessage message{0x123, {'1', '2', '3', '4', '5', '6', '7', '8'}};

    for (auto _ : state)
    {
        benchmarkObject.canRawHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));
    }
}
BENCHMARK(BM_CanRawHandler_Receive_ParseMessage_FullFrame);

static void BM_CanRawHandler_Receive_ParseMessage_ShortFrame(benchmark::State& state)
{
    CanRawHandlerBenchmark benchmarkObject;
    const CanMessage message{0x123, {'1', '2', '3'}};

    for (auto _ : state)
    {
        benchmarkObject.canRawHandler->parseReceivedMessage(&message, std::chrono::nanoseconds(0));
    }
}
BENCHMARK(BM_CanRawHandler_Receive_ParseMessage_ShortFrame);
