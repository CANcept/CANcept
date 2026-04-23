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

#include "scheduled_item_queue.hpp"

namespace Sending {

void ScheduledItemQueue::push(ScheduledItem item)
{
    bool itemAdded = false;
    {
        std::lock_guard lock(m_mutex);

        if (m_items.size() >= Constants::QUEUE_MAX_CAPACITY)
        {
            // Queue is full: evict the most overdue item (earliest scheduledAt = heap top)
            // so fresh items always replace stale ones. Semaphore count is unchanged.
            m_items.pop();
        } else
        {
            itemAdded = true;
        }

        m_items.push(std::move(item));
    }

    if (itemAdded)
    {
        m_semaphore.release();
    }
}

auto ScheduledItemQueue::pop() -> std::optional<ScheduledItem>
{
    m_semaphore.acquire();

    std::lock_guard lock(m_mutex);

    if (m_items.empty())
    {
        return std::nullopt;
    }

    ScheduledItem item = m_items.top();  // copy: safe, shared_ptr refcount managed correctly
    m_items.pop();
    return item;
}

void ScheduledItemQueue::interrupt()
{
    m_semaphore.release();
}

}  // namespace Sending