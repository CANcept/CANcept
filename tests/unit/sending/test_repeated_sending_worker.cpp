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
#include <QTest>
#include <atomic>

#include "sending/constants.hpp"
#include "sending/worker/repeated_producer_worker.hpp"

using namespace Sending;

/**
 * @brief Unit tests for RepeatedProducerWorker.
 */
class RepeatedProducerWorkerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        queue = {};
        worker = std::make_unique<RepeatedProducerWorker>(*queue);
    }

    void TearDown() override
    {
        if (worker && worker->isRunning())
        {
            worker->stopCreating();
        }
        worker.reset();
    }

    std::unique_ptr<RepeatedProducerWorker> worker;
    std::unique_ptr<ScheduledItemQueue> queue;
};

TEST_F(RepeatedProducerWorkerTest, ConstructsSuccessfully)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(RepeatedProducerWorkerTest, InitiallyNotSending)
{
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, isRunningReturnsFalseInitially)
{
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, RejectsNullCallback)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    worker->startCreating(nullptr, 100);

    ASSERT_EQ(errorSpy.count(), 1);
    const auto args = errorSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), Constants::ERR_INVALID_CALLBACK);
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, RejectsZeroInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };
    worker->startCreating(callback, 0);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, RejectsNegativeInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };
    worker->startCreating(callback, -100);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, StopWhenNotRunningIsNoOp)
{
    EXPECT_FALSE(worker->isRunning());
    EXPECT_NO_THROW(worker->stopCreating());
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, UpdateIntervalIgnoresZero)
{
    EXPECT_NO_THROW(worker->updateInterval(0));
}

TEST_F(RepeatedProducerWorkerTest, UpdateIntervalIgnoresNegative)
{
    EXPECT_NO_THROW(worker->updateInterval(-50));
}

TEST_F(RepeatedProducerWorkerTest, UpdateIntervalAcceptsPositive)
{
    EXPECT_NO_THROW(worker->updateInterval(100));
    EXPECT_NO_THROW(worker->updateInterval(500));
}

TEST_F(RepeatedProducerWorkerTest, DestructorWhenNotStartedIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

TEST_F(RepeatedProducerWorkerTest, DestructorIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

TEST_F(RepeatedProducerWorkerTest, ConstructsWithParent)
{
    QObject parent;
    const auto workerWithParent = std::make_unique<RepeatedProducerWorker>(*queue, &parent);

    EXPECT_NE(workerWithParent, nullptr);
    EXPECT_EQ(workerWithParent->parent(), &parent);

    if (workerWithParent->isRunning())
    {
        workerWithParent->stopCreating();
    }
}

TEST_F(RepeatedProducerWorkerTest, AcceptsLambdaCallback)
{
    const QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    int value = 0;
    auto callback = [&value](Clock::time_point) -> std::vector<ScheduledItem> {
        value++;
        return {};
    };
    worker->startCreating(callback, 100);

    EXPECT_EQ(errorSpy.count(), 0);
}

TEST_F(RepeatedProducerWorkerTest, ApiCallsDoNotCrash)
{
    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };

    EXPECT_NO_THROW(worker->startCreating(callback, 100));
    EXPECT_NO_THROW(worker->updateInterval(200));
    EXPECT_NO_THROW(worker->stopCreating());

    bool sending = false;
    EXPECT_NO_THROW(sending = worker->isRunning());
    (void)sending;
}

// ─── Parameterized: invalid intervals ────────────────────────────────────────

struct InvalidIntervalScenario {
    std::string name;
    int interval;
};

class InvalidIntervalTest : public RepeatedProducerWorkerTest,
                            public ::testing::WithParamInterface<InvalidIntervalScenario>
{
};

TEST_P(InvalidIntervalTest, RejectsInvalidInterval)
{
    const auto& [name, interval] = GetParam();

    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };
    worker->startCreating(callback, interval);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.takeFirst().at(0).toString(), Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isRunning());
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

class ValidIntervalTest : public RepeatedProducerWorkerTest,
                          public ::testing::WithParamInterface<ValidIntervalScenario>
{
};

TEST_P(ValidIntervalTest, AcceptsValidInterval)
{
    const auto& [name, interval] = GetParam();

    const QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };
    worker->startCreating(callback, interval);

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

TEST_F(RepeatedProducerWorkerTest, ExecutesCallback)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount](Clock::time_point) -> std::vector<ScheduledItem> {
        ++callCount;
        return {};
    };

    worker->startCreating(callback, 50);
    QTest::qWait(200);
    worker->stopCreating();

    // 200ms / 50ms interval → at least 3 executions
    EXPECT_GE(callCount.load(), 3);
}

TEST_F(RepeatedProducerWorkerTest, StartsAndStopsSuccessfully)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount](Clock::time_point) -> std::vector<ScheduledItem> {
        ++callCount;
        return {};
    };
    EXPECT_FALSE(worker->isRunning());

    worker->startCreating(callback, 50);
    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 0);

    worker->stopCreating();
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(RepeatedProducerWorkerTest, HandlesExceptionInCallback)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount](Clock::time_point) -> std::vector<ScheduledItem> {
        ++callCount;
        if (callCount == 2)
        {
            throw std::runtime_error("Test exception");
        }
        return {};
    };

    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    worker->startCreating(callback, 30);
    ASSERT_TRUE(errorSpy.wait(500));

    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("Test exception"));
    QTest::qWait(100);
    EXPECT_GT(callCount.load(), 2);
}

TEST_F(RepeatedProducerWorkerTest, UpdatesIntervalWhileRunning)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount](Clock::time_point) -> std::vector<ScheduledItem> {
        ++callCount;
        return {};
    };
    worker->startCreating(callback, 100);
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

TEST_F(RepeatedProducerWorkerTest, ErrorOnAlreadyRunning)
{
    auto callback = [](Clock::time_point) -> std::vector<ScheduledItem> { return {}; };
    const QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    worker->startCreating(callback, 100);
    QTest::qWait(50);
    worker->startCreating(callback, 200);

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

TEST_F(RepeatedProducerWorkerTest, SafeCallbackStateChanges)
{
    std::atomic<bool> shouldExecute{true};
    std::atomic<int> executionCount{0};

    auto callback = [&shouldExecute,
                     &executionCount](Clock::time_point) -> std::vector<ScheduledItem> {
        if (shouldExecute.load())
        {
            ++executionCount;
        }
        return {};
    };

    worker->startCreating(callback, 30);

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

TEST_F(RepeatedProducerWorkerTest, HandlesUnknownException)
{
    std::atomic<int> callCount{0};
    auto callback = [&callCount](Clock::time_point) -> std::vector<ScheduledItem> {
        ++callCount;
        if (callCount == 2)
        {
            throw 42;
        }
        return {};
    };

    QSignalSpy errorSpy(worker.get(), &RepeatedProducerWorker::errorOccurred);

    worker->startCreating(callback, 30);
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
