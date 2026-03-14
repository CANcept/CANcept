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
        benchmarkObject.canRawHandler->parseReceivedMessage(&message);
    }
}
BENCHMARK(BM_CanRawHandler_Receive_ParseMessage_FullFrame);

static void BM_CanRawHandler_Receive_ParseMessage_ShortFrame(benchmark::State& state)
{
    CanRawHandlerBenchmark benchmarkObject;
    const CanMessage message{0x123, {'1', '2', '3'}};

    for (auto _ : state)
    {
        benchmarkObject.canRawHandler->parseReceivedMessage(&message);
    }
}
BENCHMARK(BM_CanRawHandler_Receive_ParseMessage_ShortFrame);
