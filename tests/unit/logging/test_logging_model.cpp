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

#include "logging/model/logging_model.hpp"

using ::testing::_;

class LoggingModelTestBase : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        model = std::make_unique<Logging::LoggingModel>();
        // Create 2 test sessions
        model->startNewDbcLogSession();
        model->stopActiveSession();
        model->startNewRawLogsSession();
        model->stopActiveSession();
    }

    std::unique_ptr<Logging::LoggingModel> model;
};

TEST_F(LoggingModelTestBase, RowCount)
{
    QModelIndex invalidParent = model->index(0, 0);
    EXPECT_EQ(model->rowCount(invalidParent), 0)
        << "Row count should be 0 for invalid parent index";

    QModelIndex validParent = QModelIndex();
    EXPECT_EQ(model->rowCount(validParent), 2)
        << "Row count should match the number of created sessions (2)";
}

TEST_F(LoggingModelTestBase, ColumnCount)
{
    QModelIndex invalidParent = model->index(0, 0);
    EXPECT_EQ(model->columnCount(invalidParent), 0)
        << "Column count should be 0 for invalid parent index";

    QModelIndex validParent = QModelIndex();
    EXPECT_EQ(model->columnCount(validParent), Logging::LoggingModel::Col_MAX)
        << "Column count should match the defined number of columns (Col_MAX)";
}

TEST_F(LoggingModelTestBase, HeaderData)
{
    // Test horizontal header data for each column
    EXPECT_EQ(model->headerData(Logging::LoggingModel::Col_Timestamp, Qt::Horizontal).toString(),
              "Timestamp");
    EXPECT_EQ(model->headerData(Logging::LoggingModel::Col_Duration, Qt::Horizontal).toString(),
              "Duration");
    EXPECT_EQ(model->headerData(Logging::LoggingModel::Col_Signals, Qt::Horizontal).toString(),
              "Message-Id Signal");
    EXPECT_EQ(model->headerData(Logging::LoggingModel::Col_Actions, Qt::Horizontal).toString(),
              "Actions");

    // Test that vertical header data returns an empty QVariant
    EXPECT_FALSE(model->headerData(0, Qt::Vertical).isValid())
        << "Vertical header data should be invalid (empty)";
}

TEST_F(LoggingModelTestBase, GetSession)
{
    // Get the session IDs of the created sessions
    QString sessionId1 = model->sessionIdAt(model->index(0, 0));
    QString sessionId2 = model->sessionIdAt(model->index(1, 0));

    // Test that getSession returns valid pointers for existing session IDs
    const Logging::LogSession* session1 = model->getSession(sessionId1);
    const Logging::LogSession* session2 = model->getSession(sessionId2);
    EXPECT_NE(session1, nullptr)
        << "getSession should return a valid pointer for existing session ID";
    EXPECT_NE(session2, nullptr)
        << "getSession should return a valid pointer for existing session ID";

    // Test that the returned sessions have the correct IDs
    EXPECT_EQ(session1->id, sessionId1) << "Returned session should have the correct ID";
    EXPECT_EQ(session2->id, sessionId2) << "Returned session should have the correct ID";

    // Test that getSession returns nullptr for a non-existent session ID
    QString nonExistentSessionId = model->sessionIdAt(model->index(2, 0));
    EXPECT_EQ(model->getSession(nonExistentSessionId), nullptr)
        << "getSession should return nullptr for non-existent session ID";
}

TEST_F(LoggingModelTestBase, SessionIdAt)
{
    // 1. Create a valid index for the first row
    QModelIndex firstRowIndex = model->index(0, 0);

    // 2. Get the ID using sessionIdAt
    QString idFromMethod = model->sessionIdAt(firstRowIndex);

    // 3. Get the ID using the data() method with SessionIdRole
    QString idFromData =
        model->data(firstRowIndex, Logging::LoggingModel::SessionIdRole).toString();

    // Verify they match and aren't empty
    EXPECT_FALSE(idFromMethod.isEmpty());
    EXPECT_EQ(idFromMethod, idFromData)
        << "sessionIdAt should match the ID returned by data(SessionIdRole)";

    // 4. Test an out-of-bounds index (Row 2 doesn't exist yet)
    QModelIndex oobIndex = model->index(2, 0);
    EXPECT_TRUE(model->sessionIdAt(oobIndex).isEmpty())
        << "sessionIdAt should return an empty string for out-of-bounds index";

    // 5. Test a completely invalid index
    EXPECT_TRUE(model->sessionIdAt(QModelIndex()).isEmpty())
        << "sessionIdAt should return an empty string for a default-constructed index";
}

TEST_F(LoggingModelTestBase, IsRecording)
{
    // Initially, no session should be active
    EXPECT_FALSE(model->isRecording()) << "No session should be active initially";

    // Start a new session and check that it is active
    model->startNewDbcLogSession();
    EXPECT_TRUE(model->isRecording()) << "Session should be active after starting a new session";

    // Stop the active session and check that no session is active
    model->stopActiveSession();
    EXPECT_FALSE(model->isRecording()) << "No session should be active after stopping the session";
}

TEST_F(LoggingModelTestBase, GetCurrentSessionId)
{
    // 1. Initially, no active session (SetUp stopped them all)
    EXPECT_TRUE(model->getCurrentSessionId().isEmpty())
        << "Current session ID should be empty when no session is active";

    // 2. Start a new session
    // Note: If your signature requires maps, pass empty ones: {}, {}
    model->startNewDbcLogSession({});

    QString currentSessionId = model->getCurrentSessionId();
    EXPECT_FALSE(currentSessionId.isEmpty());

    // FIX: Compare against the LAST row, not the first
    int lastRow = model->rowCount() - 1;
    EXPECT_EQ(currentSessionId, model->sessionIdAt(model->index(lastRow, 0)))
        << "Current session ID should match the ID of the newly started session";

    // 3. Stop the active session
    model->stopActiveSession();
    EXPECT_TRUE(model->getCurrentSessionId().isEmpty())
        << "Current session ID should be empty after stopping the session";
}

TEST_F(LoggingModelTestBase, HeaderDataEdgeCases)
{
    // Test invalid role
    EXPECT_FALSE(
        model->headerData(Logging::LoggingModel::Col_Timestamp, Qt::Horizontal, Qt::UserRole)
            .isValid());

    // Test invalid orientation
    EXPECT_FALSE(
        model->headerData(Logging::LoggingModel::Col_Timestamp, Qt::Vertical, Qt::DisplayRole)
            .isValid());

    // Test out of bounds section
    EXPECT_FALSE(model->headerData(999, Qt::Horizontal, Qt::DisplayRole).isValid());
}

TEST_F(LoggingModelTestBase, DataMethodRolesAndColumns)
{
    // 1. Test Invalid Index
    EXPECT_FALSE(model->data(QModelIndex(), Qt::DisplayRole).isValid());
    EXPECT_FALSE(model->data(model->index(999, 0), Qt::DisplayRole).isValid());

    // Setup: Start DBC Session to check its specific data formatting
    // Renamed 'signals' to 'testSignalsMap' to avoid Qt macro conflicts
    std::map<uint32_t, QStringList> testSignalsMap = {{0x123, {"Sig1", "Sig2"}}};
    model->startNewDbcLogSession(testSignalsMap);
    QModelIndex dbcRow = model->index(model->rowCount() - 1, 0);

    // 2. Test Qt::DisplayRole for all columns on DBC Session
    EXPECT_TRUE(model
                    ->data(model->index(dbcRow.row(), Logging::LoggingModel::Col_Timestamp),
                           Qt::DisplayRole)
                    .isValid());
    EXPECT_EQ(
        model
            ->data(model->index(dbcRow.row(), Logging::LoggingModel::Col_Duration), Qt::DisplayRole)
            .toString(),
        "00:00:00");

    QStringList expectedDbcSignals{"0x123"};
    EXPECT_EQ(
        model->data(model->index(dbcRow.row(), Logging::LoggingModel::Col_Signals), Qt::DisplayRole)
            .toStringList(),
        expectedDbcSignals);
    EXPECT_FALSE(
        model->data(model->index(dbcRow.row(), Logging::LoggingModel::Col_Actions), Qt::DisplayRole)
            .isValid());
    EXPECT_FALSE(
        model->data(model->index(dbcRow.row(), 999), Qt::DisplayRole).isValid());  // Invalid column

    // 3. Test other Custom Roles on DBC Session
    EXPECT_TRUE(model->data(dbcRow, Logging::LoggingModel::IsActiveRole).toBool());
    EXPECT_EQ(model->data(dbcRow, Logging::LoggingModel::EntryCountRole).toULongLong(), 0);
    EXPECT_EQ(model->data(dbcRow, Logging::LoggingModel::SignalsListRole).toStringList(),
              expectedDbcSignals);
    EXPECT_FALSE(model->data(dbcRow, Qt::UserRole + 999).isValid());  // Unknown role

    // 4. Setup: Start RAW Session to check its specific data formatting
    model->startNewRawLogsSession();
    QModelIndex rawRow = model->index(model->rowCount() - 1, 0);

    // Test DisplayRole and SignalsListRole for RAW
    QStringList expectedRaw{"Raw"};
    EXPECT_EQ(
        model->data(model->index(rawRow.row(), Logging::LoggingModel::Col_Signals), Qt::DisplayRole)
            .toStringList(),
        expectedRaw);
    EXPECT_EQ(model->data(rawRow, Logging::LoggingModel::SignalsListRole).toStringList(),
              expectedRaw);
}

#include <iostream>

TEST_F(LoggingModelTestBase, DbcConfigLookups)
{
    // 1. Initial State Check (No DBC)
    uint16_t testMsgId = 0x1A4;
    QString fallback = model->getMessageName(testMsgId);

    // This will print to the console during 'ctest -V'
    std::cout << "\n[DEBUG] Fallback result: " << fallback.toStdString() << std::endl;
    EXPECT_EQ(fallback, "0x1A4");

    // 2. Setup DBC
    Core::DbcConfig mockConfig;
    Core::DbcMessageDescription msgDef;
    msgDef.messageId = testMsgId;
    msgDef.messageName = "EngineData";

    Core::DbcSignalDescription sigDef;
    sigDef.signalName = "EngineSpeed";
    sigDef.unit = "rpm";

    msgDef.signalDescriptions.push_back(sigDef);
    mockConfig.messageDefinitions.push_back(msgDef);

    model->updateDbcConfig(mockConfig);

    // 3. DBC Hit Check
    QString nameResult = model->getMessageName(testMsgId);
    std::cout << "[DEBUG] DBC Name result: " << nameResult.toStdString() << std::endl;
    EXPECT_EQ(nameResult, "EngineData");

    QString unitResult = model->getSignalUnit(testMsgId, "EngineSpeed");
    std::cout << "[DEBUG] DBC Unit result: " << unitResult.toStdString() << "\n" << std::endl;
    EXPECT_EQ(unitResult, "rpm");
}

TEST_F(LoggingModelTestBase, GetSelectedSignalsForMessage)
{
    uint16_t targetMsgId = 0x100;
    QStringList expectedSignals = {"SigA", "SigB"};
    std::map<uint32_t, QStringList> signalsMap = {{targetMsgId, expectedSignals}};

    // 1. Test when no session is active
    EXPECT_TRUE(model->getSelectedSignalsForMessage(targetMsgId).isEmpty());

    // 2. Start session and test finding the existing signal
    model->startNewDbcLogSession(signalsMap);
    EXPECT_EQ(model->getSelectedSignalsForMessage(targetMsgId), expectedSignals);

    // 3. Test asking for a message ID that wasn't selected
    EXPECT_TRUE(model->getSelectedSignalsForMessage(0x999).isEmpty());
}
/*
TEST_F(LoggingModelTestBase, DurationAndUpdateSignals)
{
    QSignalSpy dataChangedSpy(model.get(), &Logging::LoggingModel::dataChanged);

    // 1. Ensure no signal when idle
    model->updateActiveDuration();
    EXPECT_EQ(dataChangedSpy.count(), 0);

    // 2. Start a session
    model->startNewRawLogsSession();
    int activeRow = model->rowCount() - 1;  // This is row 2
    dataChangedSpy.clear();

    // 3. Test updateActiveDuration
    model->updateActiveDuration();
    ASSERT_GT(dataChangedSpy.count(), 0);

    QList<QVariant> updateArgs = dataChangedSpy.takeFirst();
    QModelIndex updateTopLeft = updateArgs.at(0).toModelIndex();
    EXPECT_EQ(updateTopLeft.row(), activeRow);
    EXPECT_EQ(updateTopLeft.column(), Logging::LoggingModel::Col_Duration);

    // 4. Test stopActiveSession
    // Based on your error log, your stopActiveSession emits a signal
    // starting at the active row and duration column.
    dataChangedSpy.clear();
    model->stopActiveSession();

    ASSERT_GT(dataChangedSpy.count(), 0);
    QList<QVariant> stopArgs = dataChangedSpy.at(0);
    QModelIndex stopTopLeft = stopArgs.at(0).toModelIndex();

    // Matching your actual output: Row 2, Column 1
    EXPECT_EQ(stopTopLeft.row(), activeRow);
    EXPECT_EQ(stopTopLeft.column(), Logging::LoggingModel::Col_Duration);
}
*/