#include "benchmark/benchmark.h"
#include "monitoring/model/monitoring_model.hpp"
#include "tests/helpers/dbc_examples.hpp"

static void BM_MonitoringModel_ChangeDbc(benchmark::State& state)
{
    Monitoring::MonitoringModel model;
    for (auto _ : state)
    {
        model.onDbcChange(TestHelpers::DbcExamples::longDbc());
    }
}
BENCHMARK(BM_MonitoringModel_ChangeDbc);

static void BM_MonitoringModel_InsertDbcMessage(benchmark::State& state)
{
    Monitoring::MonitoringModel model;
    model.onDbcChange(TestHelpers::DbcExamples::longDbc());
    Core::DbcCanMessage message = {};
    message.messageId = 100;
    message.signalValues = {{"Value100", 0}, {"Status100", 0}};
    for (auto _ : state)
    {
        model.onIncomingDbcFrame(message);
    }
}
BENCHMARK(BM_MonitoringModel_InsertDbcMessage);

static void BM_MonitoringModel_InsertDbcMessageManySignals(benchmark::State& state)
{
    Monitoring::MonitoringModel model;
    model.onDbcChange(TestHelpers::DbcExamples::manySignalMessage());
    Core::DbcCanMessage message = {};
    message.messageId = 0x100;
    for (int i = 0; i < 20; i++)
    {
        message.signalValues.push_back({"Signal" + std::to_string(i), 0});
    }
    for (auto _ : state)
    {
        model.onIncomingDbcFrame(message);
    }
}
BENCHMARK(BM_MonitoringModel_InsertDbcMessageManySignals);
