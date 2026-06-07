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
#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "core/interface/i_event_broker.hpp"
#include "event_broker/event_broker.hpp"
#include "logging/logging_component.hpp"
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
    Core::RawCanMessage encoded;
    const Core::EncodeCanMessageDbcEvent event(message, encoded);
    const Core::SendCanMessageRawEvent messageEvent(encoded);

    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
        benchmarkObject.eventBroker->publish(messageEvent);
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
    Core::RawCanMessage encoded;
    const Core::EncodeCanMessageDbcEvent event(message, encoded);
    const Core::SendCanMessageRawEvent messageEvent(encoded);

    for (auto _ : state)
    {
        benchmarkObject.eventBroker->publish(event);
        benchmarkObject.eventBroker->publish(messageEvent);
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

    benchmarkObject.eventBroker->publish(Core::AppStoppedEvent{});
    auto dbc = TestHelpers::DbcExamples::manySignalMessage();

    auto monitoringComponent =
        std::make_unique<Monitoring::MonitoringComponent>(*benchmarkObject.eventBroker);

    auto loggingComponent =
        std::make_unique<Logging::LoggingComponent>(*benchmarkObject.eventBroker);

    benchmarkObject.eventBroker->publish(Core::AppStartedEvent{});

    benchmarkObject.eventBroker->publish(Core::DBCParsedEvent(dbc, ""));

    std::map<uint32_t, QStringList> selectedSignals = {};
    QStringList signalList = {};
    for (auto signal : dbc.messageDefinitions.front().signalDescriptions)
    {
        signalList.append(QString().fromStdString(signal.signalName));
    }
    selectedSignals[(dbc.messageDefinitions.front().messageId)] = signalList;
    emit loggingComponent->getView()->startRequested(Logging::DBC_BASED, {selectedSignals});

    static int messageCount = 0;
    Core::Connection messageCountConn =
        benchmarkObject.eventBroker->subscribe<Core::ReceivedCanDbcEvent>(
            [](const Core::ReceivedCanDbcEvent& event) { messageCount++; });
    constexpr Core::RawCanMessage rawMessage = {.data = {'1', '2', '3', '4', '5', '6', '7', '8'},
                                                .messageId = 0x100};
    const Core::SendCanMessageRawEvent sendEvent(rawMessage);
    const int MESSAGE_COUNT = 100;
    for (auto _ : state)
    {
        messageCount = 0;
        state.PauseTiming();
        auto start = std::chrono::high_resolution_clock::now();
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
                if (std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::high_resolution_clock::now().time_since_epoch())
                        .count() >
                    (std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch())
                         .count() +
                     2000000000LL))
                {
                    std::cout << "Error, received no message" << std::endl;
                    return;
                }
            }
            state.ResumeTiming();
        }
        while (messageCount < MESSAGE_COUNT)
        {
            if (std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch())
                    .count() >
                (std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch())
                     .count() +
                 2000000000LL))
            {
                std::cout << "Error, received no message" << std::endl;
                return;
            }
        }
    }
}
BENCHMARK(BM_CanCommunicationHandler_Receive_ParseMessage);