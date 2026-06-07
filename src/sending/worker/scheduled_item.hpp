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

#include <chrono>
#include <memory>

namespace Sending {

using Clock = std::chrono::high_resolution_clock;

/**
 * @struct ScheduledItem
 * @brief A single unit of work to be executed at a precise point in time.
 *
 * onSend is a raw function pointer (stateless, no heap allocation per item).
 * All mutable state is held in context, which is shared across all items
 * produced in the same session — O(1) memory regardless of queue depth.
 */
struct ScheduledItem {
    Clock::time_point scheduledAt;
    void (*onSend)(void*) = nullptr;
    std::shared_ptr<void> context;
};

}  // namespace Sending