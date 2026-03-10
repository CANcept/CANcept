#include "benchmark/benchmark.h"
#include "logging/model/logging_model.hpp"
#include "tests/helpers/dbc_examples.hpp"

static void BM_LoggingModel_ChangeDbc(benchmark::State& state)
{
    Logging::LoggingModel model;
    for (auto _ : state)
    {
        model.updateDbcConfig(TestHelpers::DbcExamples::longDbc());
    }
}
BENCHMARK(BM_LoggingModel_ChangeDbc);