#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

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

/**
 * @brief Test that worker constructs successfully.
 */
TEST_F(RepeatedSendingWorkerTest, ConstructsSuccessfully)
{
    EXPECT_NE(worker, nullptr);
}

/**
 * @brief Test that worker is not sending initially.
 */
TEST_F(RepeatedSendingWorkerTest, InitiallyNotSending)
{
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test error handling for null callback.
 */
TEST_F(RepeatedSendingWorkerTest, RejectsNullCallback)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    worker->startSending(nullptr, 100);

    ASSERT_EQ(errorSpy.count(), 1);
    const auto args = errorSpy.takeFirst();
    const QString errorMsg = args.at(0).toString();
    EXPECT_EQ(errorMsg, Constants::ERR_INVALID_CALLBACK);
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test error handling for zero interval.
 */
TEST_F(RepeatedSendingWorkerTest, RejectsZeroInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, 0);

    ASSERT_EQ(errorSpy.count(), 1);
    const auto args = errorSpy.takeFirst();
    const QString errorMsg = args.at(0).toString();
    EXPECT_EQ(errorMsg, Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test error handling for negative interval.
 */
TEST_F(RepeatedSendingWorkerTest, RejectsNegativeInterval)
{
    QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    auto callback = []() {};
    worker->startSending(callback, -100);

    ASSERT_EQ(errorSpy.count(), 1);
    const auto args = errorSpy.takeFirst();
    const QString errorMsg = args.at(0).toString();
    EXPECT_EQ(errorMsg, Constants::ERR_INVALID_INTERVAL);
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test that stopSending when not running is safe (no-op).
 */
TEST_F(RepeatedSendingWorkerTest, StopWhenNotRunningIsNoOp)
{
    EXPECT_FALSE(worker->isSending());
    EXPECT_NO_THROW(worker->stopSending());
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Test that updateInterval with zero is ignored.
 */
TEST_F(RepeatedSendingWorkerTest, UpdateIntervalIgnoresZero)
{
    EXPECT_NO_THROW(worker->updateInterval(0));
}

/**
 * @brief Test that updateInterval with negative value is ignored.
 */
TEST_F(RepeatedSendingWorkerTest, UpdateIntervalIgnoresNegative)
{
    EXPECT_NO_THROW(worker->updateInterval(-50));
}

/**
 * @brief Test that updateInterval with positive value succeeds.
 */
TEST_F(RepeatedSendingWorkerTest, UpdateIntervalAcceptsPositive)
{
    EXPECT_NO_THROW(worker->updateInterval(100));
    EXPECT_NO_THROW(worker->updateInterval(500));
}

/**
 * @brief Test destructor is safe when not started.
 */
TEST_F(RepeatedSendingWorkerTest, DestructorWhenNotStartedIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

/**
 * @brief Test isSending returns false initially.
 */
TEST_F(RepeatedSendingWorkerTest, IsSendingReturnsFalseInitially)
{
    EXPECT_FALSE(worker->isSending());
}

/**
 * @brief Parameterized test for invalid interval values.
 */
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
    const auto args = errorSpy.takeFirst();
    const QString errorMsg = args.at(0).toString();
    EXPECT_EQ(errorMsg, Constants::ERR_INVALID_INTERVAL);
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

/**
 * @brief Parameterized test for valid interval values.
 */
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

    // Should not emit error for valid interval
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

/**
 * @brief Test construction with parent.
 */
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

/**
 * @brief Test that worker accepts lambda callback.
 */
TEST_F(RepeatedSendingWorkerTest, AcceptsLambdaCallback)
{
    const QSignalSpy errorSpy(worker.get(), &RepeatedSendingWorker::errorOccurred);

    int value = 0;
    auto callback = [&value]() { value++; };

    worker->startSending(callback, 100);

    // Should not emit validation error
    EXPECT_EQ(errorSpy.count(), 0);
}

/**
 * @brief Test API calls don't crash.
 */
TEST_F(RepeatedSendingWorkerTest, ApiCallsDoNotCrash)
{
    auto callback = []() {};

    EXPECT_NO_THROW(worker->startSending(callback, 100));
    EXPECT_NO_THROW(worker->updateInterval(200));
    EXPECT_NO_THROW(worker->stopSending());

    // isSending() returns a value - capture it to avoid [[nodiscard]] warning
    bool sending = false;
    EXPECT_NO_THROW(sending = worker->isSending());
    (void)sending;  // Suppress unused variable warning
}

/**
 * @brief Test that destructor is safe.
 */
TEST_F(RepeatedSendingWorkerTest, DestructorIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}
