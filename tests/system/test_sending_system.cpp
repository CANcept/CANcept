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
#include <unistd.h>

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
#include "core/widgets/common/styled_switch.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "event_broker/event_broker.hpp"
#include "math/service/variable_registry.hpp"
#include "sending/sending_component.hpp"
#include "sending/view/components/repeated_sending_card.hpp"
#include "sending/view/dbc_based_sending_subview.hpp"
#include "sending/view/raw_sending_subview.hpp"
#include "sending/view/sending_view.hpp"
#include "tests/helpers/socket_can_device_manager.hpp"
#include "tests/helpers/temp_dbc_file.hpp"

class SendingSystemTest : public ::testing::Test
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
        variableRegistry = std::make_unique<Math::VariableRegistry>(*broker);
        sending = std::make_unique<Sending::SendingComponent>(*broker, variableRegistry.get());

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

        sending.reset();
        variableRegistry.reset();
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

    auto getSendingView() const -> Sending::SendingView*
    {
        return qobject_cast<Sending::SendingView*>(sending->getView());
    }

    std::unique_ptr<TestHelpers::SocketCanDeviceManager> deviceManager;
    std::unique_ptr<EventBroker::EventBroker> broker;
    std::unique_ptr<CanHandler::CanCommunicationHandler> canHandler;
    std::unique_ptr<CanHandler::DbcHandler> dbcHandler;
    std::unique_ptr<Math::VariableRegistry> variableRegistry;
    std::unique_ptr<Sending::SendingComponent> sending;
    bool vcanCreated = false;
};

TEST_F(SendingSystemTest, FillRawForm_ClickSend_ReceivedEventHasCorrectPayload)
{
    Core::RawCanMessage receivedMsg{};
    auto rawConn = broker->subscribe<Core::ReceivedCanRawEvent>(
        [&](const Core::ReceivedCanRawEvent& e) { receivedMsg = e.canMessage; });

    connectVcan();

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);

    const auto* rawView = view->rawSubView();
    rawView->canIdEditor()->setText("042");
    rawView->messageDataEditor()->setText("AB CD");
    QTest::qWait(50);

    rawView->sendButton()->click();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_EQ(receivedMsg.messageId, 0x42) << "Received message ID should match the entered value";
    EXPECT_EQ(static_cast<uint8_t>(receivedMsg.data[0]), 0xAB) << "First data byte should be 0xAB";
    EXPECT_EQ(static_cast<uint8_t>(receivedMsg.data[1]), 0xCD) << "Second data byte should be 0xCD";
}

TEST_F(SendingSystemTest, EnableRepeatedSending_ClickSend_MultipleRawEventsFireWithinInterval)
{
    connectVcan();

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);

    auto* rawView = view->rawSubView();
    rawView->canIdEditor()->setText("042");
    rawView->messageDataEditor()->setText("AB CD");
    QTest::qWait(50);

    // Enable repeated sending toggle
    auto* repeatedCard = rawView->repeatedSendingCard();
    ASSERT_NE(repeatedCard, nullptr);
    auto* toggleSwitch = repeatedCard->findChild<Core::StyledSwitch*>();
    ASSERT_NE(toggleSwitch, nullptr) << "Toggle switch not found in RepeatedSendingCard";
    toggleSwitch->setChecked(true);
    QTest::qWait(30);
    ASSERT_TRUE(repeatedCard->isRepeatedSendingEnabled())
        << "Repeated sending not enabled after setChecked";

    // Set interval to 200ms
    repeatedCard->frequencyEditor()->setText("200");
    QTest::qWait(30);

    std::atomic<int> rawEventCount{0};
    auto rawConn = broker->subscribe<Core::ReceivedCanRawEvent>(
        [&](const Core::ReceivedCanRawEvent&) { ++rawEventCount; });

    // Click send button
    rawView->sendButton()->click();
    QTest::qWait(30);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    QTest::qWait(50);

    // which should amount to atleast 3
    const int countAfterRunning = rawEventCount.load();
    EXPECT_GE(countAfterRunning, 3)
        << "Expected at least 3 repeated messages in 1.1 seconds at 200ms interval";

    // and finally stop
    rawView->sendButton()->click();
    QTest::qWait(50);

    const int countAfterStop = rawEventCount.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    QTest::qWait(50);

    EXPECT_LE(rawEventCount.load() - countAfterStop, 1)
        << "Events should stop accumulating after repeated sending is stopped";
}

TEST_F(SendingSystemTest, FillDbcForm_ClickSend_ReceivedDbcEventHasCorrectMessageAndSignal)
{
    loadDbc(TestHelpers::makeTempDbcFile());

    bool dbcSent = false;
    auto sentConn = broker->subscribe<Core::SendCanMessageDbcEvent>(
        [&](const Core::SendCanMessageDbcEvent&) { dbcSent = true; });

    Core::DbcCanMessage receivedMsg{};
    auto dbcConn = broker->subscribe<Core::ReceivedCanDbcEvent>(
        [&](const Core::ReceivedCanDbcEvent& e) { receivedMsg = e.canMessage; });

    connectVcan();

    auto* view = getSendingView();
    ASSERT_NE(view, nullptr);
    view->displayMode(1);
    QTest::qWait(50);

    auto* dbcView = view->dbcSubView();
    const auto cards = dbcView->findChildren<Core::DbcMessageCard*>();
    ASSERT_FALSE(cards.isEmpty()) << "No DBC message cards found after loading DBC";

    cards.first()->headerCheckbox()->setChecked(true);
    QTest::qWait(50);

    ASSERT_TRUE(dbcView->sendButton()->isEnabled())
        << "DBC send button not enabled after selecting message";

    dbcView->sendButton()->click();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    QTest::qWait(50);

    EXPECT_TRUE(dbcSent) << "SendCanMessageDbcEvent was never published";
    EXPECT_EQ(receivedMsg.messageId, static_cast<uint16_t>(TestHelpers::kTestMsgId))
        << "Received DBC message ID should match kTestMsgId";
    ASSERT_FALSE(receivedMsg.signalValues.empty()) << "Received DBC message should contain signals";
    const bool hasTestSignal =
        std::any_of(receivedMsg.signalValues.begin(), receivedMsg.signalValues.end(),
                    [](const Core::DbcCanSignal& s) {
                        return s.name == TestHelpers::kTestSignalName.toStdString();
                    });
    EXPECT_TRUE(hasTestSignal) << "Received DBC event should contain signal \"TestSignal\"";
}
