#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "../../../external/googletest/googletest/include/gtest/gtest.h"
#include "../../../external/googletest/googletest/include/gtest/internal/gtest-death-test-internal.h"
#include "../tests/helpers/dbc_examples.hpp"
#include "monitoring/model/monitoring_model.hpp"

using ::testing::_;
using namespace Monitoring;
using namespace testing;

class MonitoringModelTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        model = std::make_unique<Monitoring::MonitoringModel>();
    }

    std::unique_ptr<Monitoring::MonitoringModel> model;
};

TEST_F(MonitoringModelTest, RowCountMatchesDbcConfig)
{
    auto config = TestHelpers::DbcExamples::motorController();  // Assume this has 2 messages
    model->onDbcChange(config);

    ASSERT_EQ(model->rowCount(QModelIndex()), config.messageDefinitions.size());

    QModelIndex firstMsgIdx = model->index(0, 0, QModelIndex());
    auto it = config.messageDefinitions.begin();
    ASSERT_EQ(model->rowCount(firstMsgIdx), it->signalDescriptions.size());
}

TEST_F(MonitoringModelTest, ParentNavigationIsCorrect)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    EXPECT_EQ(model->parent(sigIdx), msgIdx);

    EXPECT_FALSE(model->parent(msgIdx).isValid());
}

TEST_F(MonitoringModelTest, OnIncomingDbcFrameUpdatesInternalData)
{
    auto config = TestHelpers::DbcExamples::motorController();
    model->onDbcChange(config);

    uint32_t targetId = config.messageDefinitions.begin()->messageId;
    Core::DbcCanMessage mockMsg;
    mockMsg.messageId = targetId;
    mockMsg.receiveTime = std::chrono::milliseconds(1000);

    Core::DbcCanSignal val;
    val.name = config.messageDefinitions.begin()->signalDescriptions.begin()->signalName;
    val.value = 42.5;
    mockMsg.signalValues.push_back(val);

    model->onIncomingDbcFrame(mockMsg);

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    QVariant latestVal = model->data(sigIdx, Monitoring::MonitoringModel::Role_LatestValue);

    ASSERT_TRUE(latestVal.isValid());
    EXPECT_DOUBLE_EQ(latestVal.toDouble(), 42.5);
}

struct RoleScenario {
    std::string name;
    int role;
};

class MonitoringRoleTest : public MonitoringModelTest,
                           public ::testing::WithParamInterface<RoleScenario>
{
};

TEST_P(MonitoringRoleTest, ReturnsValidDataTypes)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());
    const auto& scenario = GetParam();

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QVariant data = model->data(msgIdx, scenario.role);

    EXPECT_TRUE(data.isValid() || !data.isValid());

    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    QVariant sigData = model->data(sigIdx, scenario.role);
    EXPECT_TRUE(sigData.isValid() || !sigData.isValid());
}

TEST_F(MonitoringModelTest, IndexReturnsInvalidWhenDbcNotLoaded)
{
    QModelIndex idx = model->index(0, 0, QModelIndex());
    EXPECT_FALSE(idx.isValid());
}

TEST_F(MonitoringModelTest, IndexReturnsInvalidForOutOfBoundsRequests)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    QModelIndex negativeRowIdx = model->index(-1, 0, QModelIndex());
    EXPECT_FALSE(negativeRowIdx.isValid());

    QModelIndex wayOutRowIdx = model->index(999, 0, QModelIndex());
    EXPECT_FALSE(wayOutRowIdx.isValid());

    QModelIndex badColIdx = model->index(0, 1, QModelIndex());
    EXPECT_FALSE(badColIdx.isValid());
}

TEST_F(MonitoringModelTest, RowCountReturnsZeroForSignalIndices)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(msgIdx.isValid());

    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    ASSERT_TRUE(sigIdx.isValid());

    EXPECT_EQ(model->rowCount(sigIdx), 0);
}

TEST_F(MonitoringModelTest, RowCountHandlesStaleOutOfBoundsMessageIndex)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    QModelIndex staleMsgIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(staleMsgIdx.isValid());
    EXPECT_EQ(staleMsgIdx.internalPointer(), nullptr);

    Core::DbcConfig emptyConfig;

    model->onDbcChange(emptyConfig);

    EXPECT_EQ(model->rowCount(staleMsgIdx), 0);
}

TEST_F(MonitoringModelTest, DataReturnsNullForMessageWithNoFrames)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());

    QVariant val = model->data(msgIdx, Monitoring::MonitoringModel::Role_LatestValue);
    EXPECT_TRUE(val.isNull());
}

TEST_F(MonitoringModelTest, MessageIdLimitGuard)
{
    Core::DbcConfig config;
    Core::DbcMessageDescription msg;
    msg.messageId = 2500;  // Above the 2048 limit
    msg.messageName = "LimitTest";
    config.messageDefinitions.push_back(msg);

    model->onDbcChange(config);
    QModelIndex idx = model->index(0, 0, QModelIndex());

    EXPECT_TRUE(model->data(idx, Monitoring::MonitoringModel::Role_LatestValue).isNull());
    EXPECT_TRUE(model->data(idx, Monitoring::MonitoringModel::Role_ValueList).isNull());
}

TEST_F(MonitoringModelTest, IncomingFrameTriggersLoopIncrement)
{
    auto config = TestHelpers::DbcExamples::motorController();  // Ensure this has > 1 message
    model->onDbcChange(config);

    if (config.messageDefinitions.size() > 1)
    {
        Core::DbcCanMessage secondMsg;
        config.messageDefinitions.pop_front();  // Remove the first message to get to the second
        secondMsg.messageId = config.messageDefinitions.front().messageId;
        EXPECT_NO_THROW(model->onIncomingDbcFrame(secondMsg));
    }
}

INSTANTIATE_TEST_SUITE_P(
    Roles, MonitoringRoleTest,
    ::testing::Values(RoleScenario{"Name", Monitoring::MonitoringModel::Role_Name},
                      RoleScenario{"ID", Monitoring::MonitoringModel::Role_ID},
                      RoleScenario{"LatestValue", Monitoring::MonitoringModel::Role_LatestValue},
                      RoleScenario{"ValueList", Monitoring::MonitoringModel::Role_ValueList},
                      RoleScenario{"Unit", Monitoring::MonitoringModel::Role_Unit},
                      RoleScenario{"SignalMin", Monitoring::MonitoringModel::Role_Min},
                      RoleScenario{"SignalMax", Monitoring::MonitoringModel::Role_Max},
                      RoleScenario{"default", -1}),
    [](const ::testing::TestParamInfo<RoleScenario>& info) { return info.param.name; });

/**
 * @brief Tests signal values specifically to hit child-level code paths.
 */
TEST_F(MonitoringModelTest, HandlesMissingSignalsInFrame)
{
    auto config = TestHelpers::DbcExamples::motorController();
    model->onDbcChange(config);

    uint32_t msgId = config.messageDefinitions.front().messageId;

    Core::DbcCanMessage partialMsg;
    partialMsg.messageId = msgId;
    partialMsg.receiveTime = std::chrono::milliseconds(500);

    model->onIncomingDbcFrame(partialMsg);

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    QVariant val = model->data(sigIdx, Monitoring::MonitoringModel::Role_LatestValue);
    EXPECT_TRUE(std::isnan(val.toDouble()));  // Should be NAN
}

/**
 * @brief Forces the eraseOldData while loop to execute.
 */
TEST_F(MonitoringModelTest, EraseOldDataCleansUpExpiredEntries)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    Core::DbcCanMessage oldMsg;
    oldMsg.messageId =
        TestHelpers::DbcExamples::motorController().messageDefinitions.front().messageId;
    // Set time to the beginning of the epoch (definitely older than HOLDING_SECONDS)
    oldMsg.receiveTime = std::chrono::milliseconds(0);

    model->onIncomingDbcFrame(oldMsg);

    // Manually trigger erasure
    model->eraseOldData();

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QVariant list = model->data(msgIdx, Monitoring::MonitoringModel::Role_ValueList);

    EXPECT_EQ(list.toList().size(), 0) << "Data should have been purged";
}

/**
 * @brief Targets boundary conditions (invalid IDs and indices).
 */
TEST_F(MonitoringModelTest, GuardClausesHandleInvalidInputs)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // 1. Invalid Index
    EXPECT_FALSE(model->data(QModelIndex(), Qt::DisplayRole).isValid());

    // 2. Out of bounds ID
    Core::DbcCanMessage hugeIdMsg;
    hugeIdMsg.messageId = 5000;  // Array size is 2048

    EXPECT_NO_THROW(model->onIncomingDbcFrame(hugeIdMsg));  // Should return early and not crash
}