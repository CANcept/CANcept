#include <filesystem>
#include <fstream>

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

auto createTempFile(const int messageCount) -> std::filesystem::path
{
    const auto path =
        std::filesystem::temp_directory_path() /
        std::filesystem::path("canbusmanager_file_parser_test_" +
                              std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                              ".tmp");
    std::ofstream file(path);
    file << R"(
VERSION "Generated DBC"

NS_ :
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SGTYPE_
    SGTYPE_VAL_
    BA_DEF_SGTYPE_
    BA_SGTYPE_
    SIG_TYPE_REF_
    VAL_TABLE_
    SIG_GROUP_
    SIG_VALTYPE_
    SIGTYPE_VALTYPE_
    BO_TX_BU_
    BA_DEF_REL_
    BA_REL_
    BA_DEF_DEF_REL_
    BU_SG_REL_
    BU_EV_REL_
    BU_BO_REL_
    SG_MUL_VAL_

BS_:

BU_: ECU1 ECU2
    )";

    for (int i = 0; i < messageCount; i++)
    {
        file << "BO_ " << i << " Msg" << i << ": 8 Vector__XXX\n";
        for (int j = 0; j < 8; j++)
        {
            file << " SG_ Signal" << j << " : " << j * 8 << "|" << 8
                 << "@1+ (1,0) [0|255] \"\" Vector__XXX\n";
        }
    }

    file.close();
    return path;
}

static void BM_DbcHandler_Parse_ShortDbc(benchmark::State& state)
{
    DbcHandlerBenchmark dbcHandler;
    static int successCounter = 0;
    static int failCounter = 0;
    Core::Connection successConn = dbcHandler.eventBroker->subscribe<Core::DBCParsedEvent>(
        [](const Core::DBCParsedEvent& event) { successCounter++; });
    Core::Connection errorConn = dbcHandler.eventBroker->subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) { failCounter++; });
    auto file = createTempFile(20);
    for (auto _ : state)
    {
        dbcHandler.eventBroker->publish(Core::ParseDBCRequestEvent(file.string()));
    }
}
BENCHMARK(BM_DbcHandler_Parse_ShortDbc);

static void BM_DbcHandler_Parse_LongDbc(benchmark::State& state)
{
    DbcHandlerBenchmark dbcHandler;
    static int successCounter = 0;
    static int failCounter = 0;
    Core::Connection successConn = dbcHandler.eventBroker->subscribe<Core::DBCParsedEvent>(
        [](const Core::DBCParsedEvent& event) { successCounter++; });
    Core::Connection errorConn = dbcHandler.eventBroker->subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) { failCounter++; });
    auto file = createTempFile(2048);
    for (auto _ : state)
    {
        dbcHandler.eventBroker->publish(Core::ParseDBCRequestEvent(absolute(file)));
    }
}
BENCHMARK(BM_DbcHandler_Parse_LongDbc);