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
    model->startNewDbcLogSession({}, {});

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
