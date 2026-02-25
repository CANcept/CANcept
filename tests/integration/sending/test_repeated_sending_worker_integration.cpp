#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>
#include <atomic>

#include "sending/constants.hpp"
#include "sending/worker/repeated_sending_worker.hpp"

using namespace Sending;

/**
 * @brief Integration tests for RepeatedSendingWorker.
 * These tests verify actual threading behavior, callback execution, and timing.
 */
class RepeatedSendingWorkerIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        worker = std::make_unique<RepeatedSendingWorker>();
    }

    void TearDown() override
    {
        if (worker && worker->isSending())
        {
            worker->stopSending();
        }
        worker.reset();
    }

    std::unique_ptr<RepeatedSendingWorker> worker;
};

/**
 * @brief Test that worker actually executes callback.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, ExecutesCallback)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() { ++callCount; };

    worker->startSending(callback, 50);

    // Wait for multiple executions
    QTest::qWait(200);

    worker->stopSending();

    // Should have executed at least 3 times (200ms / 50ms)
    EXPECT_GE(callCount.load(), 3);
}

/**
 * @brief Test that worker actually starts and stops.
 * Note: Signals are emitted from worker thread, so we verify behavior not signals.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, StartsAndStopsSuccessfully)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() { ++callCount; };

    EXPECT_FALSE(worker->isSending());

    worker->startSending(callback, 50);

    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 0);

    worker->stopSending();
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test exception handling in callback.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, HandlesExceptionInCallback)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() {
        ++callCount;
        if (callCount == 2)
        {
            throw std::runtime_error("Test exception");
        }
    };

    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(callback, 30);
    ASSERT_TRUE(errorSpy.wait(500));

    const auto args = errorSpy.takeFirst();
    const QString errorMsg = args.at(0).toString();
    EXPECT_TRUE(errorMsg.contains("Test exception"));

    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 2);

    worker->stopSending();
}

/**
 * @brief Test interval update while running.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, UpdatesIntervalWhileRunning)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() { ++callCount; };

    worker->startSending(callback, 100);
    QTest::qWait(300);

    const int countSlow = callCount.load();
    // Should execute ~2-3 times in 300ms with 100ms interval
    EXPECT_GE(countSlow, 1);
    EXPECT_LE(countSlow, 5);

    // Update to fast interval
    callCount.store(0);
    worker->updateInterval(30);
    QTest::qWait(200);

    const int countFast = callCount.load();
    // Should execute ~5-7 times in 200ms with 30ms interval
    EXPECT_GE(countFast, 4);

    worker->stopSending();
}

/**
 * @brief Test error when starting already running worker.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, ErrorOnAlreadyRunning)
{
    auto callback = []() {};

    const QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(callback, 100);
    QTest::qWait(50);

    // Try to start again
    worker->startSending(callback, 200);

    EXPECT_GE(errorSpy.count(), 1);

    bool foundError = false;
    for (int i = 0; i < errorSpy.count(); ++i)
    {
        if (const QString errorMsg = errorSpy.at(i).at(0).toString();
            errorMsg == Constants::ERR_WORKER_ALREADY_RUNNING)
        {
            foundError = true;
            break;
        }
    }

    EXPECT_TRUE(foundError);

    worker->stopSending();
}

/**
 * @brief Test that callback state changes are safe.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, SafeCallbackStateChanges)
{
    std::atomic<bool> shouldExecute{true};
    std::atomic<int> executionCount{0};

    auto callback = [&shouldExecute, &executionCount]() {
        if (shouldExecute.load())
        {
            ++executionCount;
        }
    };

    worker->startSending(callback, 30);

    QTest::qWait(100);
    EXPECT_GT(executionCount.load(), 0);

    shouldExecute.store(false);
    executionCount.store(0);
    QTest::qWait(100);
    EXPECT_EQ(executionCount.load(), 0);

    shouldExecute.store(true);
    QTest::qWait(100);
    EXPECT_GT(executionCount.load(), 0);

    worker->stopSending();
}

/**
 * @brief Test that unknown exceptions (non-std::exception) are handled.
 */
TEST_F(RepeatedSendingWorkerIntegrationTest, HandlesUnknownException)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() {
        ++callCount;
        if (callCount == 2)
        {
            throw 42;
        }
    };

    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(callback, 30);

    ASSERT_TRUE(errorSpy.wait(500));

    bool foundUnknownError = false;
    for (int i = 0; i < errorSpy.count(); ++i)
    {
        const auto args = errorSpy.at(i);
        if (const QString errorMsg = args.at(0).toString();
            errorMsg.contains(Constants::ERR_UNKNOWN_CALLBACK_ERROR))
        {
            foundUnknownError = true;
            break;
        }
    }

    EXPECT_TRUE(foundUnknownError);
    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 2);

    worker->stopSending();
}
