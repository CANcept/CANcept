#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QtTest/QTest>
#include <memory>

#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "monitoring/monitoring_component.hpp"
#include "monitoring/view/monitoring_view.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace Monitoring;

class MonitoringIntegrationTest : public ::testing::Test
{
   protected:
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<MonitoringComponent> component;
    MonitoringModel* model = nullptr;

    void SetUp() override
    {
        int argc = 1;
        static char* argv[] = {(char*)"testapp", nullptr};
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        component = std::make_unique<MonitoringComponent>(mockBroker);
        component->onStart();
        model = component->getModel();
        ASSERT_NE(model, nullptr);
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
        QApplication::processEvents();
    }
};

/**
 * @brief Validates CAN data arrival in model.
 */
TEST_F(MonitoringIntegrationTest, DataCycle_ValidatesModelContent)
{
    auto config = TestHelpers::DbcExamples::simple();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();
    Core::DbcCanMessage canMsg;
    canMsg.messageId = config.messageDefinitions.front().messageId;

    Core::DbcCanSignal sig;
    sig.name = config.messageDefinitions.front().signalDescriptions.front().signalName;
    sig.value = 99.9;
    canMsg.signalValues.push_back(sig);

    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(canMsg));
    QApplication::processEvents();

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    QVariant val = model->data(sigIdx, MonitoringModel::Role_LatestValue);
    EXPECT_DOUBLE_EQ(val.toDouble(), 99.9);
}

/**
 * @brief Tests, if unknown messages are being ignored.
 */
TEST_F(MonitoringIntegrationTest, FilterLogic_IgnoresUnknownTraffic)
{
    mockBroker.triggerEvent(Core::DBCParsedEvent(TestHelpers::DbcExamples::simple(), "test.dbc"));
    QApplication::processEvents();

    int rowCountBefore = model->rowCount(QModelIndex());

    Core::DbcCanMessage unknown;
    unknown.messageId = 0x666;
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(unknown));
    QApplication::processEvents();

    EXPECT_EQ(model->rowCount(QModelIndex()), rowCountBefore);
}

/**
 * @brief Tests, if loading a new DBC clears old data.
 */
TEST_F(MonitoringIntegrationTest, SwitchingDbcClearsOldData)
{
    mockBroker.triggerEvent(Core::DBCParsedEvent(TestHelpers::DbcExamples::simple(), "first.dbc"));
    QApplication::processEvents();

    Core::DbcCanMessage msg;
    msg.messageId = TestHelpers::DbcExamples::simple().messageDefinitions.front().messageId;
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(msg));
    QApplication::processEvents();

    ASSERT_GT(model->rowCount(QModelIndex()), 0);

    Core::DbcConfig emptyConfig;  // Keine Nachrichten definiert
    mockBroker.triggerEvent(Core::DBCParsedEvent(emptyConfig, "empty.dbc"));
    QApplication::processEvents();

    EXPECT_EQ(model->rowCount(QModelIndex()), 0)
        << "Model was not cleared after loading a new DBC configuration!";
}

/**
 * @brief Tests, if signal updates correctly update existing rows instead of adding new ones.
 */
TEST_F(MonitoringIntegrationTest, SignalUpdatesCorrectlyWithinExistingRow)
{
    auto config = TestHelpers::DbcExamples::simple();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    auto msgId = config.messageDefinitions.front().messageId;
    auto sigName = config.messageDefinitions.front().signalDescriptions.front().signalName;

    Core::DbcCanMessage msg1;
    msg1.messageId = msgId;
    msg1.signalValues.push_back({sigName, 10.0});
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(msg1));

    Core::DbcCanMessage msg2;
    msg2.messageId = msgId;
    msg2.signalValues.push_back({sigName, 20.0});
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(msg2));

    QApplication::processEvents();

    ASSERT_EQ(model->rowCount(QModelIndex()), 1);

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    EXPECT_DOUBLE_EQ(model->data(sigIdx, MonitoringModel::Role_LatestValue).toDouble(), 20.0);
}

TEST_F(MonitoringIntegrationTest, HandlesEmptySignalListGracefully)
{
    auto config = TestHelpers::DbcExamples::simple();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    Core::DbcCanMessage emptyMsg;
    emptyMsg.messageId = config.messageDefinitions.front().messageId;
    emptyMsg.signalValues.clear();

    EXPECT_NO_THROW({
        mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(emptyMsg));
        QApplication::processEvents();
    });

    SUCCEED();
}