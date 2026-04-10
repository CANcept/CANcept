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
#include "core/event/dbc_event.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/logging_component.hpp"
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

class LoggingComponentBenchmark
{
   public:
    LoggingComponentBenchmark()
    {
        eventBroker = std::make_unique<EventBroker::EventBroker>();
        component = std::make_unique<Logging::LoggingComponent>(*eventBroker);
        eventBroker->publish(Core::AppStartedEvent{});
    }

    ~LoggingComponentBenchmark()
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        component.reset();
        eventBroker.reset();
    }

    std::unique_ptr<Core::IEventBroker> eventBroker;
    std::unique_ptr<Logging::LoggingComponent> component;
};

static void BM_LoggingComponent_DbcSession(benchmark::State& state)
{
    const LoggingComponentBenchmark benchmark;
    const Core::DbcConfig dbc = TestHelpers::DbcExamples::longDbc();
    benchmark.eventBroker->publish(Core::DBCParsedEvent(dbc, "test.dbc"));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::map<uint32_t, QStringList> selectedSignals = {};
    QStringList signalList = {};
    for (auto signal : dbc.messageDefinitions.front().signalDescriptions)
    {
        signalList.append(QString().fromStdString(signal.signalName));
    }
    selectedSignals[(dbc.messageDefinitions.front().messageId)] = signalList;
    emit benchmark.component->getView()->startRequested(Logging::DBC_BASED, selectedSignals);

    const auto message = Core::DbcCanMessage{
        .signalValues = {{dbc.messageDefinitions.front().signalDescriptions.front().signalName,
                          42.0}},
        .messageId = static_cast<uint16_t>(dbc.messageDefinitions.front().messageId)};
    const auto event = Core::ReceivedCanDbcEvent(message);
    for (auto _ : state)
    {
        benchmark.eventBroker->publish(event);
    }
}
BENCHMARK(BM_LoggingComponent_DbcSession);

static void BM_LoggingComponent_RawSession(benchmark::State& state)
{
    const LoggingComponentBenchmark benchmark;
    const Core::DbcConfig dbc = TestHelpers::DbcExamples::longDbc();
    benchmark.eventBroker->publish(Core::DBCParsedEvent(dbc, "test.dbc"));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    emit benchmark.component->getView()->startRequested(Logging::RAW, {});

    const auto message = Core::RawCanMessage{
        .data = {}, .messageId = static_cast<uint16_t>(dbc.messageDefinitions.front().messageId)};
    const auto event = Core::ReceivedCanRawEvent(message);

    for (auto _ : state)
    {
        benchmark.eventBroker->publish(event);
    }
}
BENCHMARK(BM_LoggingComponent_RawSession);