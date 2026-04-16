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

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include "sending/constants.hpp"
#include "sending/worker/scheduled_item.hpp"
#include "sending/worker/scheduled_item_queue.hpp"

using namespace Sending;

namespace {

ScheduledItem makeItem(Clock::time_point when)
{
    ScheduledItem item;
    item.scheduledAt = when;
    item.onSend = nullptr;
    return item;
}

}  // namespace

/**
 * @brief Unit tests for ScheduledItemQueue.
 */
class ScheduledItemQueueTest : public ::testing::Test
{
   protected:
    ScheduledItemQueue queue;
};

// ─── Basic push / pop ────────────────────────────────────────────────────────

TEST_F(ScheduledItemQueueTest, PopReturnsItemAfterPush)
{
    const auto deadline = Clock::now() + std::chrono::milliseconds(100);
    queue.push(makeItem(deadline));

    const auto result = queue.pop();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->scheduledAt, deadline);
}

TEST_F(ScheduledItemQueueTest, PopBlocksUntilItemPushed)
{
    std::atomic<bool> popped{false};

    std::thread popper([&] {
        queue.pop();
        popped.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    EXPECT_FALSE(popped.load()) << "pop() must block while the queue is empty";

    queue.push(makeItem(Clock::now()));

    popper.join();
    EXPECT_TRUE(popped.load());
}

// ─── Priority ordering ───────────────────────────────────────────────────────

TEST_F(ScheduledItemQueueTest, ItemsReturnedInEarliestFirstOrder)
{
    const auto now = Clock::now();
    const auto t1 = now + std::chrono::milliseconds(300);
    const auto t2 = now + std::chrono::milliseconds(100);
    const auto t3 = now + std::chrono::milliseconds(200);

    queue.push(makeItem(t1));
    queue.push(makeItem(t2));
    queue.push(makeItem(t3));

    const auto first = queue.pop();
    const auto second = queue.pop();
    const auto third = queue.pop();

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    ASSERT_TRUE(third.has_value());

    EXPECT_EQ(first->scheduledAt, t2);
    EXPECT_EQ(second->scheduledAt, t3);
    EXPECT_EQ(third->scheduledAt, t1);
}

// ─── Interrupt ───────────────────────────────────────────────────────────────

TEST_F(ScheduledItemQueueTest, InterruptUnblocksEmptyPop)
{
    std::atomic<bool> returned{false};
    std::optional<ScheduledItem> result;

    std::thread popper([&] {
        result = queue.pop();
        returned.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    EXPECT_FALSE(returned.load());

    queue.interrupt();

    popper.join();
    EXPECT_TRUE(returned.load());
    EXPECT_FALSE(result.has_value()) << "Interrupted pop must return nullopt";
}

// ─── Capacity / eviction ─────────────────────────────────────────────────────

TEST_F(ScheduledItemQueueTest, AcceptsUpToMaxCapacityItems)
{
    const auto now = Clock::now();

    for (int i = 0; i < Constants::QUEUE_MAX_CAPACITY; ++i)
    {
        queue.push(makeItem(now + std::chrono::microseconds(i)));
    }

    // All pushed items must be poppable (no eviction occurred at or below capacity)
    for (int i = 0; i < Constants::QUEUE_MAX_CAPACITY; ++i)
    {
        const auto item = queue.pop();
        ASSERT_TRUE(item.has_value()) << "Item " << i << " should be present";
    }
}

TEST_F(ScheduledItemQueueTest, EvictsAtCapacityAndSemaphoreRemainsConsistent)
{
    const auto now = Clock::now();

    // Fill to capacity
    for (int i = 0; i < Constants::QUEUE_MAX_CAPACITY; ++i)
    {
        queue.push(makeItem(now + std::chrono::microseconds(i)));
    }

    // Push one more — must evict one item without incrementing the semaphore
    queue.push(makeItem(now + std::chrono::microseconds(Constants::QUEUE_MAX_CAPACITY)));

    // Exactly QUEUE_MAX_CAPACITY items should be poppable
    for (int i = 0; i < Constants::QUEUE_MAX_CAPACITY; ++i)
    {
        const auto item = queue.pop();
        ASSERT_TRUE(item.has_value()) << "Item " << i << " should be present after eviction";
    }

    // The next pop must block (no item available and no semaphore credit left)
    std::atomic<bool> blocked{true};
    std::thread popper([&] {
        // give the interrupt a moment to be called; if pop() was unblocked before
        // the interrupt that would indicate a semaphore imbalance
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        blocked.store(false);
        queue.interrupt();  // unblock so the thread can exit
        queue.pop();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(blocked.load()) << "After eviction the semaphore must not carry extra credits";

    popper.join();
}