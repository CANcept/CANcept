
#include "benchmark/benchmark.h"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "can_handler/dbc_handler/dbc_parser.hpp"
#include "core/interface/i_event_broker.hpp"
#include "tests/helpers/mock_event_broker.hpp"
class DbcHandlerBenchmark
{
   public:
    DbcHandlerBenchmark()
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        dbcHandler = std::make_unique<CanHandler::DbcHandler>(*eventBroker);
        eventBroker->publish(Core::AppStartedEvent{});
    }
    ~DbcHandlerBenchmark()
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        dbcHandler.reset();
        eventBroker.reset();
    }
    std::unique_ptr<Core::IEventBroker> eventBroker;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
};
static void BM_DbcHandler_Parse_ShortDbc(benchmark::State& state)
{
    DbcHandlerBenchmark dbcHandler;
    static int successCounter = 0;
    static int failCounter = 0;
    Core::Connection successConn = dbcHandler.eventBroker->subscribe<Core::DBCParsedEvent>(
        [](const Core::DBCParsedEvent& event) { successCounter++; });
    Core::Connection errorConn = dbcHandler.eventBroker->subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) {
            failCounter++;
            std::cout << event.errorMessage << std::endl;
        });
    for (auto _ : state)
    {
        dbcHandler.eventBroker->publish(
            Core::ParseDBCRequestEvent("./tests/assets/dbc/shortDbc.dbc"));
    }
    std::cout << successCounter << " successful parses, " << failCounter << " failed parses."
              << std::endl;
}
BENCHMARK(BM_DbcHandler_Parse_ShortDbc);

static void BM_DbcHandler_Parse_LongDbc(benchmark::State& state)
{
    DbcHandlerBenchmark dbcHandler;
    static int successCounter = 0;
    static int failCounter = 0;
    Core::Connection successConn = dbcHandler.eventBroker->subscribe<Core::DBCParsedEvent>(
        [](const Core::DBCParsedEvent& event) {
            successCounter++;
            std::cout << event.config.messageDefinitions.size() << " messages parsed." << std::endl;
        });
    Core::Connection errorConn = dbcHandler.eventBroker->subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) { failCounter++; });
    for (auto _ : state)
    {
        dbcHandler.eventBroker->publish(
            Core::ParseDBCRequestEvent("./tests/assets/dbc/longDbc.dbc"));
    }
    std::cout << successCounter << " successful parses, " << failCounter << " failed parses."
              << std::endl;
}
BENCHMARK(BM_DbcHandler_Parse_LongDbc);