#include <memory>

#include "benchmark/benchmark.h"
#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "core/interface/i_event_broker.hpp"
#include "event_broker/event_broker.hpp"
#include "monitoring/monitoring_component.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
class CanCommunicationHandlerBenchmark
{
   public:
    CanCommunicationHandlerBenchmark()
    {
        eventBroker = std::make_unique<EventBroker::EventBroker>();
        canCommunicationHandler =
            std::make_unique<CanHandler::CanCommunicationHandler>(*eventBroker);
        eventBroker->publish(Core::AppStartedEvent{});
        if (!getuid())
        {
            deviceManager.create();
            deviceManager.up();
            createdVCanDevice = true;
            eventBroker->publish(Core::CanDriverChangeEvent("vcan0"));
        }
    }
    ~CanCommunicationHandlerBenchmark()
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        canCommunicationHandler.reset();
        eventBroker.reset();
        if (createdVCanDevice)
        {
            deviceManager.down();
            deviceManager.remove();
        }
    }
    std::unique_ptr<Core::IEventBroker> eventBroker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canCommunicationHandler;
    TestHelpers::SocketCanDeviceManager deviceManager{"vcan0"};
    bool createdVCanDevice = false;
};

static void BM_CanCommunicationHandler_Send_RawMessage(benchmark::State& state)
{
    CanCommunicationHandlerBenchmark benchmarkObject;
    if (!benchmarkObject.createdVCanDevice)
    {
        state.SkipWithMessage(
            "Cannot run raw message benchmark without root permissions to create vcan device.");
        return;
    }
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
BENCHMARK(BM_CanCommunicationHandler_Send_RawMessage);

static void BM_CanCommunicationHandler_Send_DbcMessage(benchmark::State& state)
{
    const CanCommunicationHandlerBenchmark benchmarkObject;
    if (!benchmarkObject.createdVCanDevice)
    {
        state.SkipWithMessage(
            "Cannot run raw message benchmark without root permissions to create vcan device.");
        return;
    }
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::longDbc(), ""));
    Core::DbcCanMessage message = {};
    message.messageId = 0x100;
    message.signalValues = {{"Value100", 0}, {"Status100", 0}};
    const Core::SendCanMessageDbcEvent event(message);

    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanCommunicationHandler_Send_DbcMessage);

static void BM_CanCommunicationHandler_Send_DbcMessageManySignals(benchmark::State& state)
{
    const CanCommunicationHandlerBenchmark benchmarkObject;
    if (!benchmarkObject.createdVCanDevice)
    {
        state.SkipWithMessage(
            "Cannot run raw message benchmark without root permissions to create vcan device.");
        return;
    }
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::manySignalMessage(), ""));
    Core::DbcCanMessage message = {};
    message.messageId = 0x100;
    for (int i = 0; i < 20; i++)
    {
        message.signalValues.push_back({"Signal" + std::to_string(i), 0});
    }
    const Core::SendCanMessageDbcEvent event(message);

    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
    }
}
BENCHMARK(BM_CanCommunicationHandler_Send_DbcMessageManySignals);

static void BM_CanCommunicationHandler_Receive_ParseMessage(benchmark::State& state)
{
    CanCommunicationHandlerBenchmark benchmarkObject;
    if (!benchmarkObject.createdVCanDevice)
    {
        state.SkipWithMessage(
            "Cannot run raw message benchmark without root permissions to create vcan device.");
        return;
    }
    Monitoring::MonitoringModel monitoringModel;
    Core::Connection monitoringDbcConn =
        benchmarkObject.eventBroker->subscribe<Core::DBCParsedEvent>(
            [&monitoringModel](const Core::DBCParsedEvent& event) {
                monitoringModel.onDbcChange(event.config);
            });
    Core::Connection monitoringMessageConn =
        benchmarkObject.eventBroker->subscribe<Core::ReceivedCanDbcEvent>(
            [&monitoringModel](const Core::ReceivedCanDbcEvent& event) {
                monitoringModel.onIncomingDbcFrame(event.canMessage);
            });
    benchmarkObject.eventBroker->publish(
        Core::DBCParsedEvent(TestHelpers::DbcExamples::longDbc(), ""));
    static int messageCount = 0;
    Core::Connection messageCountConn =
        benchmarkObject.eventBroker->subscribe<Core::ReceivedCanDbcEvent>(
            [](const Core::ReceivedCanDbcEvent& event) { messageCount++; });
    constexpr Core::RawCanMessage rawMessage = {.data = {'1', '2', '3', '4', '5', '6', '7', '8'},
                                                .messageId = 100};
    const Core::SendCanMessageRawEvent sendEvent(rawMessage);
    const int MESSAGE_COUNT = 100;
    for (auto _ : state)
    {
        messageCount = 0;
        state.PauseTiming();
        int i = 0;
        for (; i < MESSAGE_COUNT; i++)
        {
            benchmarkObject.eventBroker->publish(sendEvent);
            if (messageCount > 0)
            {
                state.ResumeTiming();
                break;
            }
        }
        for (; i < MESSAGE_COUNT; i++)
        {
            benchmarkObject.eventBroker->publish(sendEvent);
        }
        if (messageCount == 0)
        {
            while (messageCount == 0)
            {
            }
            state.ResumeTiming();
        }
        while (messageCount < MESSAGE_COUNT)
        {
        }
    }
}
BENCHMARK(BM_CanCommunicationHandler_Receive_ParseMessage);