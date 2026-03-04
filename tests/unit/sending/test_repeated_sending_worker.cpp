#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>
#include <atomic>

#include "sending/constants.hpp"
#include "sending/worker/repeated_sending_worker.hpp"

using namespace Sending;

/**
 * @brief Unit tests for RepeatedSendingWorker.
 */
class RepeatedSendingWorkerTest : public ::testing::Test
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

TEST_F(RepeatedSendingWorkerTest, ConstructsSuccessfully)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(RepeatedSendingWorkerTest, InitiallyNotSending)
{
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, IsSendingReturnsFalseInitially)
{
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, RejectsNullCallback)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(nullptr, 100);

    ASSERT_EQ(errorSpy.count(), 1);
    const auto args = errorSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), Constants::ERR_INVALID_CALLBACK);
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, RejectsZeroInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, 0);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, RejectsNegativeInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, -100);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, StopWhenNotRunningIsNoOp)
{
    EXPECT_FALSE(worker->isSending());
    EXPECT_NO_THROW(worker->stopSending());
    EXPECT_FALSE(worker->isSending());
}

TEST_F(RepeatedSendingWorkerTest, UpdateIntervalIgnoresZero)
{
    EXPECT_NO_THROW(worker->updateInterval(0));
}

TEST_F(RepeatedSendingWorkerTest, UpdateIntervalIgnoresNegative)
{
    EXPECT_NO_THROW(worker->updateInterval(-50));
}

TEST_F(RepeatedSendingWorkerTest, UpdateIntervalAcceptsPositive)
{
    EXPECT_NO_THROW(worker->updateInterval(100));
    EXPECT_NO_THROW(worker->updateInterval(500));
}

TEST_F(RepeatedSendingWorkerTest, DestructorWhenNotStartedIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

TEST_F(RepeatedSendingWorkerTest, DestructorIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

TEST_F(RepeatedSendingWorkerTest, ConstructsWithParent)
{
    QObject parent;
    const auto workerWithParent = std::make_unique<RepeatedSendingWorker>(&parent);

    EXPECT_NE(workerWithParent, nullptr);
    EXPECT_EQ(workerWithParent->parent(), &parent);

    if (workerWithParent->isSending())
    {
        workerWithParent->stopSending();
    }
}

TEST_F(RepeatedSendingWorkerTest, AcceptsLambdaCallback)
{
    const QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    int value = 0;
    auto callback = [&value]() { value++; };
    worker->startSending(callback, 100);

    EXPECT_EQ(errorSpy.count(), 0);
}

TEST_F(RepeatedSendingWorkerTest, ApiCallsDoNotCrash)
{
    auto callback = []() {};

    EXPECT_NO_THROW(worker->startSending(callback, 100));
    EXPECT_NO_THROW(worker->updateInterval(200));
    EXPECT_NO_THROW(worker->stopSending());

    bool sending = false;
    EXPECT_NO_THROW(sending = worker->isSending());
    (void)sending;
}

// ─── Parameterized: invalid intervals ────────────────────────────────────────

struct InvalidIntervalScenario {
    std::string name;
    int interval;
};

class InvalidIntervalTest : public RepeatedSendingWorkerTest,
                            public ::testing::WithParamInterface<InvalidIntervalScenario>
{
};

TEST_P(InvalidIntervalTest, RejectsInvalidInterval)
{
    const auto& [name, interval] = GetParam();

    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, interval);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isSending());
}

INSTANTIATE_TEST_SUITE_P(InvalidIntervalScenarios, InvalidIntervalTest,
                         ::testing::Values(InvalidIntervalScenario{"Zero", 0},
                                           InvalidIntervalScenario{"Negative_10", -10},
                                           InvalidIntervalScenario{"Negative_100", -100},
                                           InvalidIntervalScenario{"Negative_1000", -1000}),
                         [](const ::testing::TestParamInfo<InvalidIntervalScenario>& info) {
                             return info.param.name;
                         });

// ─── Parameterized: valid intervals ──────────────────────────────────────────

struct ValidIntervalScenario {
    std::string name;
    int interval;
};

class ValidIntervalTest : public RepeatedSendingWorkerTest,
                          public ::testing::WithParamInterface<ValidIntervalScenario>
{
};

TEST_P(ValidIntervalTest, AcceptsValidInterval)
{
    const auto& [name, interval] = GetParam();

    const QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, interval);

    EXPECT_EQ(errorSpy.count(), 0);
}

INSTANTIATE_TEST_SUITE_P(ValidIntervalScenarios, ValidIntervalTest,
                         ::testing::Values(ValidIntervalScenario{"VeryFast_1ms", 1},
                                           ValidIntervalScenario{"Fast_10ms", 10},
                                           ValidIntervalScenario{"Medium_50ms", 50},
                                           ValidIntervalScenario{"Default_100ms", 100},
                                           ValidIntervalScenario{"Slow_500ms", 500},
                                           ValidIntervalScenario{"VerySlow_1000ms", 1000}),
                         [](const ::testing::TestParamInfo<ValidIntervalScenario>& info) {
                             return info.param.name;
                         });

// ─── Threading / timing tests ────────────────────────────────────────────────

TEST_F(RepeatedSendingWorkerTest, ExecutesCallback)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() { ++callCount; };

    worker->startSending(callback, 50);
    QTest::qWait(200);
    worker->stopSending();

    // 200ms / 50ms interval → at least 3 executions
    EXPECT_GE(callCount.load(), 3);
}

TEST_F(RepeatedSendingWorkerTest, StartsAndStopsSuccessfully)
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

TEST_F(RepeatedSendingWorkerTest, HandlesExceptionInCallback)
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

    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("Test exception"));
    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 2);
}

TEST_F(RepeatedSendingWorkerTest, UpdatesIntervalWhileRunning)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount]() { ++callCount; };

    worker->startSending(callback, 100);
    QTest::qWait(300);

    const int countSlow = callCount.load();
    EXPECT_GE(countSlow, 1);
    EXPECT_LE(countSlow, 5);

    callCount.store(0);
    worker->updateInterval(30);
    QTest::qWait(200);

    // ~5–7 executions in 200ms at 30ms interval
    EXPECT_GE(callCount.load(), 4);
}

TEST_F(RepeatedSendingWorkerTest, ErrorOnAlreadyRunning)
{
    auto callback = []() {};
    const QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(callback, 100);
    QTest::qWait(50);
    worker->startSending(callback, 200);

    EXPECT_GE(errorSpy.count(), 1);

    bool foundError = false;
    for (int i = 0; i < errorSpy.count(); ++i)
    {
        if (errorSpy.at(i).at(0).toString() == Constants::ERR_WORKER_ALREADY_RUNNING)
        {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

TEST_F(RepeatedSendingWorkerTest, SafeCallbackStateChanges)
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
}

TEST_F(RepeatedSendingWorkerTest, HandlesUnknownException)
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
        if (errorSpy.at(i).at(0).toString().contains(Constants::ERR_UNKNOWN_CALLBACK_ERROR))
        {
            foundUnknownError = true;
            break;
        }
    }
    EXPECT_TRUE(foundUnknownError);
    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 2);
}
