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
