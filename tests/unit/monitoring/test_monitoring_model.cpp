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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "external/googletest/googletest/include/gtest/gtest.h"
#include "external/googletest/googletest/include/gtest/internal/gtest-death-test-internal.h"
#include "monitoring/model/monitoring_model.hpp"
#include "tests/helpers/dbc_examples.hpp"

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
    auto config = TestHelpers::DbcExamples::motorController();
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

TEST_P(MonitoringRoleTest, ReturnsExpectedValidity)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());
    const auto& scenario = GetParam();

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QVariant msgData = model->data(msgIdx, scenario.role);

    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    QVariant sigData = model->data(sigIdx, scenario.role);

    if (scenario.role == -1)
    {
        EXPECT_FALSE(msgData.isValid());
        EXPECT_FALSE(sigData.isValid());
    } else if (scenario.role == Monitoring::MonitoringModel::Role_LatestValue)
    {
        EXPECT_FALSE(msgData.isValid()) << "LatestValue should be invalid before first frame";
        EXPECT_FALSE(sigData.isValid()) << "LatestValue should be invalid before first frame";
    } else if (scenario.role == Monitoring::MonitoringModel::Role_ValueList)
    {
        EXPECT_TRUE(msgData.isValid())
            << "ValueList should be valid (empty container) after DBC load";
        EXPECT_TRUE(sigData.isValid())
            << "ValueList should be valid (empty container) after DBC load";
    } else
    {
        if (scenario.role == Monitoring::MonitoringModel::Role_Unit ||
            scenario.role == Monitoring::MonitoringModel::Role_Min ||
            scenario.role == Monitoring::MonitoringModel::Role_Max)
        {
            EXPECT_FALSE(msgData.isValid());
        } else
        {
            EXPECT_TRUE(msgData.isValid());
        }

        if (scenario.role == Monitoring::MonitoringModel::Role_ID)
        {
            EXPECT_FALSE(sigData.isValid());
        } else
        {
            EXPECT_TRUE(sigData.isValid());
        }
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
    auto config = TestHelpers::DbcExamples::motorController();
    model->onDbcChange(config);

    if (config.messageDefinitions.size() > 1)
    {
        Core::DbcCanMessage secondMsg;
        config.messageDefinitions.pop_front();
        secondMsg.messageId = config.messageDefinitions.front().messageId;
        EXPECT_NO_THROW(model->onIncomingDbcFrame(secondMsg));
    }
}

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
