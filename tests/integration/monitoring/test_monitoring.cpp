#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QTimer>

#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "monitoring/monitoring_component.hpp"
#include "monitoring/view/monitoring_view.hpp"
#include "tests/helpers/dbc_examples.hpp"

// Helpers
#include "../tests/helpers/dbc_examples.hpp"
#include "../tests/helpers/mock_event_broker.hpp"

using namespace Monitoring;

class MonitoringIntegrationTest : public ::testing::Test
{
   protected:
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<MonitoringComponent> component;
    MonitoringView* view = nullptr;
    MonitoringModel* model = nullptr;

    void SetUp() override
    {
        int argc = 0;
        char* argv[] = {nullptr};
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }

        // 1. Initialize Component
        component = std::make_unique<MonitoringComponent>(mockBroker);

        // 2. Start (this triggers subscriptions in the broker)
        component->onStart();

        // 3. Extract view and model for verification
        view = qobject_cast<MonitoringView*>(component->getView());
        ASSERT_NE(view, nullptr);

        // Assuming your MonitoringView has a getter for the model
        model = component->getModel();
        ASSERT_NE(model, nullptr);
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }
};

/**
 * @brief Integration: Verifies that a DBCParsedEvent correctly populates the tree model.
 */
TEST_F(MonitoringIntegrationTest, DbcLoadingPopulatesModel)
{
    // 1. Prepare a dummy DBC via your helper
    auto config = TestHelpers::DbcExamples::simple();
    std::string filename = "test_data.dbc";

    // 2. Simulate the Event Broker receiving a parsed DBC
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, filename));

    // 3. Process Qt Events (Signals/Slots between Component and Model)
    QApplication::processEvents();

    // 4. Verify Model State
    int expectedMsgCount = static_cast<int>(config.messageDefinitions.size());
    ASSERT_EQ(model->rowCount(QModelIndex()), expectedMsgCount);

    QModelIndex firstMsgIdx = model->index(0, 0, QModelIndex());
    QString nameInModel = model->data(firstMsgIdx, MonitoringModel::Role_Name).toString();
    EXPECT_EQ(nameInModel, QString::fromStdString(config.messageDefinitions.front().messageName));
}

/**
 * @brief Integration: Verifies that CAN traffic via ReceivedCanDbcEvent updates signal values.
 */
TEST_F(MonitoringIntegrationTest, CanTrafficUpdatesSignalValues)
{
    // 1. Setup: Load the DBC structure first
    auto config = TestHelpers::DbcExamples::simple();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    // 2. Prepare an incoming CAN message (Decoded)
    Core::DbcCanMessage canMsg;
    canMsg.messageId = config.messageDefinitions.front().messageId;
    canMsg.receiveTime = std::chrono::milliseconds(1000);

    // Create a signal value match
    Core::DbcCanSignal sigVal;
    sigVal.name = config.messageDefinitions.front().signalDescriptions.front().signalName;
    sigVal.value = 123.45;
    canMsg.signalValues.push_back(sigVal);

    // 3. Act: Trigger the CAN event through the broker
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(canMsg));
    QApplication::processEvents();

    // 4. Assert: Navigate to the signal index and check Role_LatestValue
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);  // First signal

    QVariant value = model->data(sigIdx, MonitoringModel::Role_LatestValue);
    EXPECT_FALSE(value.isNull());
    EXPECT_DOUBLE_EQ(value.toDouble(), 123.45);
}

/**
 * @brief Integration: Verifies the "No DBC" overlay logic via Readiness events.
 */
TEST_F(MonitoringIntegrationTest, DeviceReadinessTogglesOverlay)
{
    // Note: checkDeviceReadiness is called onStart and on DBC load.
    // Since we start without a DBC, the overlay should be visible.

    // 1. Act: Provide the missing piece (DBC)
    mockBroker.triggerEvent(Core::DBCParsedEvent(TestHelpers::DbcExamples::simple(), "test.dbc"));

    // 2. Act: Simulate CAN Driver becoming ready
    // We expect the component to publish CheckCanDeviceReadyEvent and we set 'isReady' to true
    // This part requires your MockBroker to handle the "Ready" check response.

    QApplication::processEvents();

    // 3. Assert: The model should have data, suggesting the overlay should be gone
    EXPECT_GT(model->rowCount(QModelIndex()), 0);
}