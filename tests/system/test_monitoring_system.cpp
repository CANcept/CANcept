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

#include <gtest/gtest.h>
#include <qwt_plot.h>
#include <unistd.h>

#include <QPushButton>
#include <QTest>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "event_broker/event_broker.hpp"
#include "monitoring/monitoring_component.hpp"
#include "monitoring/view/graph_list_view.hpp"
#include "monitoring/view/monitoring_view.hpp"
#include "monitoring/view/signal_graph.hpp"
#include "monitoring/view/signal_list.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

class MonitoringSystemTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        if (getuid() != 0)
        {
            GTEST_SKIP() << "System tests require root privileges (run start.sh with sudo)";
        }

        deviceManager = std::make_unique<TestHelpers::SocketCanDeviceManager>("vcan0");
        try
        {
            deviceManager->create();
            deviceManager->up();
            vcanCreated = true;
        } catch (const std::exception& e)
        {
            GTEST_SKIP() << "Failed to set up vcan0: " << e.what();
        }

        broker = std::make_unique<EventBroker::EventBroker>();
        canHandler = std::make_unique<CanHandler::CanCommunicationHandler>(*broker);
        dbcHandler = std::make_unique<CanHandler::DbcHandler>(*broker);
        monitoring = std::make_unique<Monitoring::MonitoringComponent>(*broker);

        broker->publish<Core::AppStartedEvent>({});
        QTest::qWait(100);
    }

    void TearDown() override
    {
        if (broker)
        {
            broker->publish<Core::AppStoppedEvent>({});
            QTest::qWait(50);
        }

        monitoring.reset();
        dbcHandler.reset();
        canHandler.reset();
        broker.reset();

        if (vcanCreated)
        {
            deviceManager->down();
            deviceManager->remove();
        }
    }

    void connectVcan() const
    {
        broker->publish<Core::CanDriverChangeEvent>(Core::CanDriverChangeEvent{"vcan0"});
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    void loadDbc(const std::string& path) const
    {
        broker->publish<Core::ParseDBCRequestEvent>(Core::ParseDBCRequestEvent{path});
        QTest::qWait(2000);
    }

    auto getMonitoringView() const -> Monitoring::MonitoringView*
    {
        return qobject_cast<Monitoring::MonitoringView*>(monitoring->getView());
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<Monitoring::MonitoringComponent> monitoring;
    bool vcanCreated = false;
};

TEST_F(MonitoringSystemTest, ExpandSignalList_CheckSignal_GraphIsCreatedAndReceivesData)
{
    loadDbc(TestHelpers::makeTempDbcFile());
    QTest::qWait(50);

    auto* view = getMonitoringView();
    ASSERT_NE(view, nullptr);

    auto* signalList = view->getSignalListView();
    ASSERT_NE(signalList, nullptr);

    // Expand the first message card
    const auto expandButtons = signalList->findChildren<QPushButton*>();
    ASSERT_FALSE(expandButtons.isEmpty()) << "No expand button found in SignalList";
    expandButtons.first()->click();
    QTest::qWait(50);

    // and select the first signal
    const auto checkboxes = signalList->findChildren<Core::StyledCheckBox*>();
    ASSERT_FALSE(checkboxes.isEmpty())
        << "No signal checkboxes found after expanding the message card";
    checkboxes.first()->click();
    QTest::qWait(50);

    auto* graphListView = view->getGraphListView();
    ASSERT_NE(graphListView, nullptr);

    const auto graphs = graphListView->findChildren<Monitoring::SignalGraph*>();
    ASSERT_FALSE(graphs.isEmpty()) << "No SignalGraph was created after enabling signal monitoring";
    EXPECT_EQ(graphs.first()->getSignalName(), TestHelpers::kTestSignalName)
        << "Graph signal name should match the selected signal name";

    connectVcan();

    Core::RawCanMessage msg{};
    msg.messageId = TestHelpers::kTestMsgId;
    msg.dlc = 8;
    msg.data[0] = 0x55;
    broker->publish<Core::SendCanMessageRawEvent>(Core::SendCanMessageRawEvent{msg});

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    QTest::qWait(200);

    const auto* graph = graphListView->findChildren<Monitoring::SignalGraph*>().first();
    auto* plot = graph->findChild<QwtPlot*>();
    ASSERT_NE(plot, nullptr) << "QwtPlot not found inside SignalGraph";
    EXPECT_TRUE(plot->axisEnabled(QwtPlot::xBottom))
        << "QwtPlot x-axis should be enabled after decoded CAN data was pushed to the graph";
}
