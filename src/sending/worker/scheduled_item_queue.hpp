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

#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <vector>

#include "sending/constants.hpp"
#include "sending/worker/scheduled_item.hpp"

namespace Sending {

/**
 * @class ScheduledItemQueue
 * @brief Thread-safe producer-consumer channel using a priority queue.
 */
class ScheduledItemQueue
{
    using Clock = std::chrono::high_resolution_clock;

   public:
    /**
     * @brief Enqueues an item.
     * O(log N). Thread-safe.
     */
    void push(ScheduledItem item);

    /**
     * @brief Non-evicting enqueue. Returns false when the queue is at capacity so the
     * caller can spin-wait without silently dropping frames. O(log N). Thread-safe.
     */
    [[nodiscard, gnu::noinline]] auto tryPush(ScheduledItem item) -> bool;

    /**
     * @brief Blocks until an item is available, then returns the item
     * with the earliest scheduledAt time.
     * O(log N) complexity. It is thread-safe.
     */
    [[nodiscard]] auto pop() -> std::optional<ScheduledItem>;

    /**
     * @brief Unblocks a waiting pop() call.
     */
    void interrupt();

   private:
    // Custom comparator to make the priority_queue a "Min-Heap" based on time.
    struct CompareScheduledAt {
        auto operator()(const ScheduledItem& a, const ScheduledItem& b) const -> bool
        {
            return a.scheduledAt > b.scheduledAt;
        }
    };

    std::priority_queue<ScheduledItem, std::vector<ScheduledItem>, CompareScheduledAt> m_items;
    mutable std::mutex m_mutex;
    std::counting_semaphore<Constants::QUEUE_MAX_CAPACITY> m_semaphore{0};
};

}  // namespace Sending