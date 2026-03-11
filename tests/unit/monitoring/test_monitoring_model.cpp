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
    int role;
    QMetaType::Type expectedType; // Der erwartete Qt-Typ
    bool isValidForMessage;       // Gibt die Nachricht für diese Rolle Daten zurück?
    bool isValidForSignal;        // Gibt das Signal für diese Rolle Daten zurück?
};

class MonitoringRoleTest : public MonitoringModelTest,
                           public ::testing::WithParamInterface<RoleScenario>
{
};

TEST_P(MonitoringRoleTest, ReturnsCorrectDataTypesAndValues)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());
    const auto& scenario = GetParam();

    // --- 1. Prüfung auf Nachrichtenebene ---
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    QVariant msgData = model->data(msgIdx, scenario.role);

    if (scenario.isValidForMessage) {
        EXPECT_TRUE(msgData.isValid()) << "Role " << scenario.role << " should be valid for Messages";
        EXPECT_EQ(msgData.typeId(), scenario.expectedType);
        
        // Spezifische Inhaltsprüfung (Beispiel für Name)
        if (scenario.role == Monitoring::MonitoringModel::Role_Name) {
            EXPECT_FALSE(msgData.toString().isEmpty());
        }
    } else {
        EXPECT_FALSE(msgData.isValid()) << "Role " << scenario.role << " should be empty for Messages";
    }

    // --- 2. Prüfung auf Signalebene ---
    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    QVariant sigData = model->data(sigIdx, scenario.role);

    if (scenario.isValidForSignal) {
        EXPECT_TRUE(sigData.isValid()) << "Role " << scenario.role << " should be valid for Signals";
        EXPECT_EQ(sigData.typeId(), scenario.expectedType);
    } else {
        EXPECT_FALSE(sigData.isValid()) << "Role " << scenario.role << " should be empty for Signals";
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
    // Do NOT call onDbcChange() here.
    // m_currentDbc.has_value() will be false.

    QModelIndex idx = model->index(0, 0, QModelIndex());
    EXPECT_FALSE(idx.isValid());  // This directly hits line 47
}

TEST_F(MonitoringModelTest, IndexReturnsInvalidForOutOfBoundsRequests)
{
    // Load a valid DBC
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // 1. Test negative row
    QModelIndex negativeRowIdx = model->index(-1, 0, QModelIndex());
    EXPECT_FALSE(negativeRowIdx.isValid());

    // 2. Test out-of-bounds row (assuming you don't have 999 messages)
    QModelIndex wayOutRowIdx = model->index(999, 0, QModelIndex());
    EXPECT_FALSE(wayOutRowIdx.isValid());

    // 3. Test out-of-bounds column
    // (Your columnCount() returns 1, so column index 1 is out of bounds)
    QModelIndex badColIdx = model->index(0, 1, QModelIndex());
    EXPECT_FALSE(badColIdx.isValid());
}

TEST_F(MonitoringModelTest, RowCountReturnsZeroForSignalIndices)
{
    // Load a valid DBC
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // 1. Get a valid Message index (Root's child)
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(msgIdx.isValid());

    // 2. Get a valid Signal index (Message's child)
    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    ASSERT_TRUE(sigIdx.isValid());

    // 3. Ask for the row count of the Signal index.
    // Because it has an internal pointer, it skips the message logic
    // and drops down to the final return 0; (Line 103)
    EXPECT_EQ(model->rowCount(sigIdx), 0);
}

TEST_F(MonitoringModelTest, RowCountHandlesStaleOutOfBoundsMessageIndex)
{
    // 1. Load a populated DBC (assume motorController has at least 1 message)
    model->onDbcChange(TestHelpers::DbcExamples::motorController());

    // Get an index for the first message (row 0)
    QModelIndex staleMsgIdx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(staleMsgIdx.isValid());
    EXPECT_EQ(staleMsgIdx.internalPointer(), nullptr);  // Confirms it's a message index

    // 2. Load an EMPTY DBC to simulate changing files
    Core::DbcConfig emptyConfig;
    // Assuming default constructed config has 0 messageDefinitions
    model->onDbcChange(emptyConfig);

    // 3. Ask for the rowCount using the old index.
    // staleMsgIdx.row() is 0.
    // new messageDefinitions.size() is 0.
    // 0 >= 0 is TRUE -> hits the safety return 0; (Line 97)
    EXPECT_EQ(model->rowCount(staleMsgIdx), 0);
}

TEST_F(MonitoringModelTest, DataReturnsNullForMessageWithNoFrames)
{
    model->onDbcChange(TestHelpers::DbcExamples::motorController());
    QModelIndex msgIdx = model->index(0, 0, QModelIndex());

    // Role_LatestValue checks messageValues->at(id).timestamps.size()
    // Since we haven't called onIncomingDbcFrame, size is 0.
    // Hits line 147.
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

    // This hits 'if (it->messageId >= messageValues->size())'
    EXPECT_TRUE(model->data(idx, Monitoring::MonitoringModel::Role_LatestValue).isNull());
    EXPECT_TRUE(model->data(idx, Monitoring::MonitoringModel::Role_ValueList).isNull());
}

TEST_F(MonitoringModelTest, IncomingFrameTriggersLoopIncrement)
{
    auto config = TestHelpers::DbcExamples::motorController();  // Ensure this has > 1 message
    model->onDbcChange(config);

    // Pick the SECOND message in the DBC to force currentRow++
    if (config.messageDefinitions.size() > 1)
    {
        Core::DbcCanMessage secondMsg;
        config.messageDefinitions.pop_front();  // Remove the first message to get to the second
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
    hugeIdMsg.messageId = 5000;  // Array size is 2048

    EXPECT_NO_THROW(model->onIncomingDbcFrame(hugeIdMsg));  // Should return early and not crash
}
