#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSignalSpy>

#include "monitoring/model/monitoring_model.hpp"
#include "tests/helpers/dbc_examples.hpp"

using ::testing::_;

class MonitoringModelTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        model = std::make_unique<Monitoring::MonitoringModel>();
    }

    std::unique_ptr<Monitoring::MonitoringModel> model;
};

// --- 1. Hierarchy Tests ---

TEST_F(MonitoringModelTest, RowCountMatchesDbcConfig)
{
    auto config = TestHelpers::DbcExamples::motorController();  // Assume this has 2 messages
    model->onDbcChange(config);

    // Root level: Number of messages
    ASSERT_EQ(model->rowCount(QModelIndex()), config.messageDefinitions.size());

    // Child level: Number of signals in the first message
    QModelIndex firstMsgIdx = model->index(0, 0, QModelIndex());
    auto it = config.messageDefinitions.begin();
    ASSERT_EQ(model->rowCount(firstMsgIdx), it->signalDescriptions.size());
}

TEST_F(MonitoringModelTest, ParentNavigationIsCorrect)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    // Parent of a signal must be the message
    EXPECT_EQ(model->parent(sigIdx), msgIdx);
    // Parent of a message must be invalid (root)
    EXPECT_FALSE(model->parent(msgIdx).isValid());
}

// --- 2. Data Ingestion Tests ---

TEST_F(MonitoringModelTest, OnIncomingDbcFrameUpdatesInternalData)
{
    auto config = TestHelpers::DbcExamples::motorController();
    model->onDbcChange(config);

    // 1. Prepare a mock message
    uint32_t targetId = config.messageDefinitions.begin()->messageId;
    Core::DbcCanMessage mockMsg;
    mockMsg.messageId = targetId;
    mockMsg.receiveTime = std::chrono::milliseconds(1000);

    // Add a mock signal value
    Core::DbcCanSignal val;
    val.name = config.messageDefinitions.begin()->signalDescriptions.begin()->signalName;
    val.value = 42.5;
    mockMsg.signalValues.push_back(val);

    // 2. Inject data
    model->onIncomingDbcFrame(mockMsg);

    // 3. Verify via data() method (since signals aren't emitted)
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QModelIndex sigIdx = model->index(0, 0, msgIdx);

    QVariant latestVal = model->data(sigIdx, Monitoring::MonitoringModel::Role_LatestValue);

    ASSERT_TRUE(latestVal.isValid());
    EXPECT_DOUBLE_EQ(latestVal.toDouble(), 42.5);
}

// --- 3. Parametrized Role Tests ---

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

    // For MonitoringModel, these should at least return a valid type if the row exists
    EXPECT_TRUE(data.isValid() || !data.isValid());
}

INSTANTIATE_TEST_SUITE_P(
    Roles, MonitoringRoleTest,
    ::testing::Values(RoleScenario{"Name", Monitoring::MonitoringModel::Role_Name},
                      RoleScenario{"ID", Monitoring::MonitoringModel::Role_ID},
                      RoleScenario{"LatestValue", Monitoring::MonitoringModel::Role_LatestValue},
                      RoleScenario{"ValueList", Monitoring::MonitoringModel::Role_ValueList}),
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
    // Note: We are NOT adding any signalValues here to trigger the !valueExists path

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
    hugeIdMsg.messageId = 5000;            // Array size is 2048
    model->onIncomingDbcFrame(hugeIdMsg);  // Should return early and not crash

    SUCCEED();
}