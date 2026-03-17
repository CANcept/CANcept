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

#include <memory>

#include "benchmark/benchmark.h"
#include "can_handler/can_communication_handler/can_dbc_handler.hpp"
#include "core/event/lifecycle_event.hpp"
#include "event_broker/event_broker.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"
class CanDbcHandlerBenchmark
{
   public:
    CanDbcHandlerBenchmark()
    {
        eventBroker = std::make_unique<EventBroker::EventBroker>();
        canDbcHandler = std::make_unique<CanHandler::CanDbcHandler>(
            *eventBroker, [](const CanMessage&) { return true; });
        eventBroker->publish(Core::AppStartedEvent{});
    }
    ~CanDbcHandlerBenchmark()
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        canDbcHandler.reset();
        eventBroker.reset();
    }
    std::unique_ptr<Core::IEventBroker> eventBroker;
    std::unique_ptr<CanHandler::CanDbcHandler> canDbcHandler;
};

static void BM_CanDbcHandler_Send_ParseMessage_ShortDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::motorController(), "motor_controller.dbc"));
    Core::DbcCanMessage message = {};
    message.messageId = 0x100;
    message.signalValues = {{"Speed", 0}, {"Temperature", 0}, {"ErrorCode", 0}};
    Core::SendCanMessageDbcEvent event(message);
    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanDbcHandler_Send_ParseMessage_ShortDbc);

static void BM_CanDbcHandler_Send_ParseMessage_LongDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::longDbc(), ""));
    Core::DbcCanMessage message = {};
    message.messageId = 100;
    message.signalValues = {{"Value100", 0}, {"Status100", 0}};
    Core::SendCanMessageDbcEvent event(message);
    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanDbcHandler_Send_ParseMessage_LongDbc);

static void BM_CanDbcHandler_Send_ParseMessage_ManySignalsDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(Core::DBCParsedEvent(
        TestHelpers::DbcExamples::manySignalMessage(), "motor_controller.dbc"));
    Core::DbcCanMessage message = {};
    message.messageId = 0x100;
    for (int i = 0; i < 20; i++)
    {
        message.signalValues.push_back({"Signal" + std::to_string(i), 0});
    }
    Core::SendCanMessageDbcEvent event(message);
    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanDbcHandler_Send_ParseMessage_ManySignalsDbc);

static void BM_CanDbcHandler_Receive_ParseMessage_ShortDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::motorController(), "motor_controller.dbc"));
    const CanMessage message{0x100, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};
    for (auto _ : state)
    {
        benchmarkObject.canDbcHandler->parseReceivedMessage(&message);
    }
}
BENCHMARK(BM_CanDbcHandler_Receive_ParseMessage_ShortDbc);

static void BM_CanDbcHandler_Receive_ParseMessage_LongDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::longDbc(), ""));
    const CanMessage message{100, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};
    for (auto _ : state)
    {
        benchmarkObject.canDbcHandler->parseReceivedMessage(&message);
    }
}
BENCHMARK(BM_CanDbcHandler_Receive_ParseMessage_LongDbc);

static void BM_CanDbcHandler_Receive_ParseMessage_ManySignalsDbc(benchmark::State& state)
{
    CanDbcHandlerBenchmark benchmarkObject;
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::manySignalMessage(), ""));
    const CanMessage message{0x100, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};
    for (auto _ : state)
    {
        benchmarkObject.canDbcHandler->parseReceivedMessage(&message);
    }
}
BENCHMARK(BM_CanDbcHandler_Receive_ParseMessage_ManySignalsDbc);
