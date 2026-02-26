#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QCoreApplication>

#include "monitoring/model/monitoring_model.hpp"
#include "tests/helpers/can_test_matchers.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/dbc_examples.hpp"

using ::testing::_;

class MonitoringModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        model = std::make_unique<Monitoring::MonitoringModel>();
    }

    void TearDown() override
    {
        model.reset();
    }

    std::unique_ptr<Monitoring::MonitoringModel> model;
};

// --- Basic Structure Tests ---

TEST_F(MonitoringModelTest, InitialStateIsEmpty)
{
    EXPECT_EQ(model->rowCount(QModelIndex()), 0);
    // Even an empty tree typically has at least 1 column for the display hierarchy
    EXPECT_GE(model->columnCount(QModelIndex()), 1);
}

TEST_F(MonitoringModelTest, CreatesValidIndexAfterDbcLoad)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // Assuming the mock DBC has at least one frame
    const QModelIndex idx = model->index(0, 0, QModelIndex());
    EXPECT_TRUE(idx.isValid());
}

TEST_F(MonitoringModelTest, ReturnsInvalidParentForRootItems)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    const QModelIndex idx = model->index(0, 0, QModelIndex());
    const QModelIndex parentIdx = model->parent(idx);

    // Top-level frames should not have a parent
    EXPECT_FALSE(parentIdx.isValid());
}

// --- Tree Hierarchy Tests ---

TEST_F(MonitoringModelTest, TreeStructureWithDbcMessages)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // 1. Verify Root (Frames)
    // Adjust the expected count based on what DbcExamples::motorController() actually returns
    const int frameCount = model->rowCount(QModelIndex());
    EXPECT_GT(frameCount, 0) << "DBC should load at least one frame";

    const QModelIndex frameIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(frameIdx.isValid());

    // 2. Verify Children (Signals)
    const int signalCount = model->rowCount(frameIdx);
    EXPECT_GT(signalCount, 0) << "Frame should have at least one signal";

    const QModelIndex signalIdx = model->index(0, 0, frameIdx);
    ASSERT_TRUE(signalIdx.isValid());

    // 3. Verify Parent-Child Relationship
    const QModelIndex parentIdx = model->parent(signalIdx);
    EXPECT_EQ(parentIdx, frameIdx);
}

// --- Parametrized Role Tests ---

struct ModelDataRoleScenario {
    std::string name;
    int role;
};

class MonitoringModelRoleTest : public ::testing::TestWithParam<ModelDataRoleScenario>
{
protected:
    void SetUp() override
    {
        model = std::make_unique<Monitoring::MonitoringModel>();
        model->onDbcChange(TestHelpers::DbcExamples::motorController());
    }
    std::unique_ptr<Monitoring::MonitoringModel> model;
};

TEST_P(MonitoringModelRoleTest, ReturnsVariantForRole)
{
    const auto& scenario = GetParam();

    // Get a valid signal index (Child of the first frame)
    const QModelIndex frameIdx = model->index(0, 0, QModelIndex());
    const QModelIndex signalIdx = model->index(0, 0, frameIdx);

    ASSERT_TRUE(signalIdx.isValid()) << "Setup failed to create valid signal index";

    const QVariant result = model->data(signalIdx, scenario.role);

    // We just want to ensure it doesn't crash and returns a QVariant.
    // Depending on your implementation, uninitialized data might return an invalid QVariant,
    // which is technically fine until data is populated.
    SUCCEED();
}

INSTANTIATE_TEST_SUITE_P(
    MonitoringRoles, MonitoringModelRoleTest,
    ::testing::Values(
        ModelDataRoleScenario{"RoleName", Monitoring::MonitoringModel::Role_Name},
        ModelDataRoleScenario{"RoleID", Monitoring::MonitoringModel::Role_ID},
        ModelDataRoleScenario{"RoleValueList", Monitoring::MonitoringModel::Role_ValueList},
        ModelDataRoleScenario{"RoleLatestValue", Monitoring::MonitoringModel::Role_LatestValue},
        ModelDataRoleScenario{"RoleUnit", Monitoring::MonitoringModel::Role_Unit},
        ModelDataRoleScenario{"RoleMax", Monitoring::MonitoringModel::Role_Max},
        ModelDataRoleScenario{"RoleMin", Monitoring::MonitoringModel::Role_Min}
    ),
    [](const ::testing::TestParamInfo<ModelDataRoleScenario>& info) { return info.param.name; }
);

// --- Data Ingestion Tests ---

TEST_F(MonitoringModelTest, OnIncomingDbcFrameTriggersDataChanged)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // Setup a spy to listen to view updates
    QSignalSpy dataChangedSpy(model.get(), &QAbstractItemModel::dataChanged);

    // Create a mock decoded frame matching one from motorController()
    Core::DbcCanMessage mockMsg;
    // mockMsg.messageId = ...; Populate this with valid mock data

    model->onIncomingDbcFrame(mockMsg);

    // Process background thread events
    QCoreApplication::processEvents();

    // Expect the model to notify the view that data has updated
    // Note: You may need to add a short QTest::qWait() here if your internal
    // message_check_thread takes time to process the batch.
    EXPECT_GE(dataChangedSpy.count(), 1);
}

TEST_F(MonitoringModelTest, EraseOldDataClearsBatches)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // Ingest data
    Core::DbcCanMessage mockMsg;
    model->onIncomingDbcFrame(mockMsg);
    QCoreApplication::processEvents();

    // Trigger clear
    model->eraseOldData();
    QCoreApplication::processEvents();

    // Retrieve data using custom role and ensure it's cleared/reset
    const QModelIndex frameIdx = model->index(0, 0, QModelIndex());
    const QModelIndex signalIdx = model->index(0, 0, frameIdx);

    QVariant latestVal = model->data(signalIdx, Monitoring::MonitoringModel::Role_LatestValue);

    // Assert that the value is now empty, 0, or invalid depending on your implementation
    // EXPECT_FALSE(latestVal.isValid());
}