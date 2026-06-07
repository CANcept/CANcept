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
#include "sending/worker/scheduled_item.hpp"
#include "sending/worker/scheduled_item_queue.hpp"
#include "sending/worker/sending_consumer_worker.hpp"

using namespace Sending;

/**
 * @brief Unit tests for SendingConsumerWorker.
 */
class SendingConsumerWorkerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        queue = std::make_unique<ScheduledItemQueue>();
        worker = std::make_unique<SendingConsumerWorker>(*queue);
    }

    void TearDown() override
    {
        if (worker && worker->isConsuming())
        {
            worker->stopConsuming();
        }
        worker.reset();
    }

    /** Pushes an item with the given callback scheduled to fire immediately. */
    void pushNow(void (*onSend)(void*), std::shared_ptr<void> context = nullptr) const
    {
        ScheduledItem item;
        item.scheduledAt = Clock::now();
        item.onSend = onSend;
        item.context = std::move(context);
        queue->push(std::move(item));
    }

    /** Pushes an item scheduled @p offsetUs microseconds into the future. */
    void pushIn(const int offsetUs, void (*onSend)(void*),
                std::shared_ptr<void> context = nullptr) const
    {
        ScheduledItem item;
        item.scheduledAt = Clock::now() + std::chrono::microseconds(offsetUs);
        item.onSend = onSend;
        item.context = std::move(context);
        queue->push(std::move(item));
    }

    std::unique_ptr<ScheduledItemQueue> queue;
    std::unique_ptr<SendingConsumerWorker> worker;
};

// ─── Lifecycle ───────────────────────────────────────────────────────────────

TEST_F(SendingConsumerWorkerTest, ConstructsSuccessfully)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(SendingConsumerWorkerTest, InitiallyNotConsuming)
{
    EXPECT_FALSE(worker->isConsuming());
}

TEST_F(SendingConsumerWorkerTest, StartsConsuming)
{
    worker->startConsuming();
    QTest::qWait(30);

    EXPECT_TRUE(worker->isConsuming());
}

TEST_F(SendingConsumerWorkerTest, StopsConsuming)
{
    worker->startConsuming();
    QTest::qWait(30);

    worker->stopConsuming();

    EXPECT_FALSE(worker->isConsuming());
}

TEST_F(SendingConsumerWorkerTest, StopWhenNotRunningIsNoOp)
{
    EXPECT_FALSE(worker->isConsuming());
    EXPECT_NO_THROW(worker->stopConsuming());
    EXPECT_FALSE(worker->isConsuming());
}

TEST_F(SendingConsumerWorkerTest, MultipleStartStopCycles)
{
    for (int i = 0; i < 3; ++i)
    {
        worker->startConsuming();
        QTest::qWait(20);
        EXPECT_TRUE(worker->isConsuming());

        worker->stopConsuming();
        EXPECT_FALSE(worker->isConsuming());
    }
}

TEST_F(SendingConsumerWorkerTest, SecondStartWhileRunningIsNoOp)
{
    worker->startConsuming();
    QTest::qWait(20);
    EXPECT_TRUE(worker->isConsuming());

    worker->startConsuming();  // should be ignored
    EXPECT_TRUE(worker->isConsuming());
}

TEST_F(SendingConsumerWorkerTest, ConstructsWithParent)
{
    QObject parent;
    auto workerWithParent = std::make_unique<SendingConsumerWorker>(*queue, &parent);

    EXPECT_NE(workerWithParent, nullptr);
    EXPECT_EQ(workerWithParent->parent(), &parent);
}

TEST_F(SendingConsumerWorkerTest, DestructorWhenNotStartedIsSafe)
{
    EXPECT_NO_THROW(worker.reset());
}

TEST_F(SendingConsumerWorkerTest, DestructorWhileRunningIsSafe)
{
    worker->startConsuming();
    QTest::qWait(20);

    EXPECT_NO_THROW(worker.reset());
}

// ─── Signals ─────────────────────────────────────────────────────────────────

TEST_F(SendingConsumerWorkerTest, EmitsConsumingStarted)
{
    QSignalSpy spy(worker.get(), &SendingConsumerWorker::consumingStarted);

    worker->startConsuming();

    ASSERT_TRUE(spy.wait(500));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(SendingConsumerWorkerTest, EmitsConsumingStopped)
{
    worker->startConsuming();
    QTest::qWait(30);

    QSignalSpy spy(worker.get(), &SendingConsumerWorker::consumingStopped);

    worker->stopConsuming();

    QTest::qWait(100);
    EXPECT_EQ(spy.count(), 1);
}

// ─── Item consumption ────────────────────────────────────────────────────────

TEST_F(SendingConsumerWorkerTest, ConsumesItemAndExecutesCallback)
{
    std::atomic<int> callCount{0};
    auto ctx = std::make_shared<std::atomic<int>*>(&callCount);

    worker->startConsuming();
    QTest::qWait(20);

    pushNow(
        [](void* p) {
            auto* counter = *static_cast<std::atomic<int>**>(p);
            ++(*counter);
        },
        ctx);

    QTest::qWait(100);

    EXPECT_EQ(callCount.load(), 1);
}

TEST_F(SendingConsumerWorkerTest, ConsumesMultipleItems)
{
    std::atomic<int> callCount{0};
    auto ctx = std::make_shared<std::atomic<int>*>(&callCount);

    auto onSend = [](void* p) {
        auto* counter = *static_cast<std::atomic<int>**>(p);
        ++(*counter);
    };

    worker->startConsuming();
    QTest::qWait(20);

    constexpr int kItems = 5;
    for (int i = 0; i < kItems; ++i)
    {
        pushNow(onSend, ctx);
    }

    QTest::qWait(200);

    EXPECT_EQ(callCount.load(), kItems);
}

TEST_F(SendingConsumerWorkerTest, SkipsItemWithNullOnSend)
{
    worker->startConsuming();
    QTest::qWait(20);

    ScheduledItem item;
    item.scheduledAt = Clock::now();
    item.onSend = nullptr;
    queue->push(std::move(item));

    QTest::qWait(100);

    SUCCEED();
}

TEST_F(SendingConsumerWorkerTest, ExecutesCallbackWithCorrectContext)
{
    constexpr int kSentinel = 42;
    auto ctx = std::make_shared<int>(kSentinel);

    std::atomic<int> receivedValue{0};
    auto ctxWrapper =
        std::make_shared<std::pair<std::shared_ptr<int>, std::atomic<int>*>>(ctx, &receivedValue);

    worker->startConsuming();
    QTest::qWait(20);

    pushNow(
        [](void* p) {
            auto* pair = static_cast<std::pair<std::shared_ptr<int>, std::atomic<int>*>*>(p);
            pair->second->store(*pair->first);
        },
        ctxWrapper);

    QTest::qWait(100);

    EXPECT_EQ(receivedValue.load(), kSentinel);
}

// ─── Exception handling ──────────────────────────────────────────────────────

TEST_F(SendingConsumerWorkerTest, HandlesExceptionInOnSend)
{
    QSignalSpy errorSpy(worker.get(), &SendingConsumerWorker::errorOccurred);

    worker->startConsuming();
    QTest::qWait(20);

    pushNow([](void*) { throw std::runtime_error("Test send error"); });

    ASSERT_TRUE(errorSpy.wait(500));

    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("Test send error"));
    EXPECT_TRUE(worker->isConsuming()) << "Worker must keep running after a callback exception";
}

TEST_F(SendingConsumerWorkerTest, HandlesUnknownExceptionInOnSend)
{
    QSignalSpy errorSpy(worker.get(), &SendingConsumerWorker::errorOccurred);

    worker->startConsuming();
    QTest::qWait(20);

    pushNow([](void*) { throw 99; });

    ASSERT_TRUE(errorSpy.wait(500));

    EXPECT_TRUE(
        errorSpy.takeFirst().at(0).toString().contains(Constants::ERR_UNKNOWN_CALLBACK_ERROR));
    EXPECT_TRUE(worker->isConsuming()) << "Worker must keep running after an unknown exception";
}

TEST_F(SendingConsumerWorkerTest, ContinuesConsumingAfterException)
{
    std::atomic<int> callCount{0};
    auto ctx = std::make_shared<std::atomic<int>*>(&callCount);

    worker->startConsuming();
    QTest::qWait(20);

    // First item throws
    pushNow([](void*) { throw std::runtime_error("oops"); });

    QTest::qWait(50);

    // Second item must still be consumed
    pushNow(
        [](void* p) {
            auto* counter = *static_cast<std::atomic<int>**>(p);
            ++(*counter);
        },
        ctx);

    QTest::qWait(100);

    EXPECT_EQ(callCount.load(), 1);
}

// ─── Timing ──────────────────────────────────────────────────────────────────

TEST_F(SendingConsumerWorkerTest, DoesNotFireBeforeScheduledTime)
{
    std::atomic<bool> fired{false};
    auto ctx = std::make_shared<std::atomic<bool>*>(&fired);

    worker->startConsuming();
    QTest::qWait(20);

    // Schedule 150 ms in the future
    pushIn(
        150'000,
        [](void* p) {
            auto* flag = *static_cast<std::atomic<bool>**>(p);
            flag->store(true);
        },
        ctx);

    QTest::qWait(80);
    EXPECT_FALSE(fired.load()) << "Callback must not fire before the scheduled time";

    QTest::qWait(150);
    EXPECT_TRUE(fired.load()) << "Callback must fire after the scheduled deadline";
}