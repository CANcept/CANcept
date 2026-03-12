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