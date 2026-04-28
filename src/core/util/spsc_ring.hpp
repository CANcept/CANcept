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

#include <array>
#include <atomic>
#include <cstddef>

namespace Core {

/**
 * @brief Single-producer / single-consumer lock-free ring buffer.
 *
 * push() and pop() are wait-free. N must be a power of two.
 */
template <typename T, std::size_t N>
class SpscRing
{
    static_assert((N & (N - 1)) == 0, "N must be a power of two");
    static constexpr std::size_t kMask = N - 1;

   public:
    /** @brief Enqueue one item. Returns false if the ring is full. */
    [[nodiscard]] bool push(T val)
    {
        const std::size_t head = m_head.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) & kMask;
        if (next == m_tail.load(std::memory_order_acquire)) return false;
        m_ring[head] = std::move(val);
        m_head.store(next, std::memory_order_release);
        return true;
    }

    /** @brief Dequeue one item into val. Returns false if the ring is empty. */
    [[nodiscard]] bool pop(T& val)
    {
        const std::size_t tail = m_tail.load(std::memory_order_relaxed);
        if (tail == m_head.load(std::memory_order_acquire)) return false;
        val = std::move(m_ring[tail]);
        m_tail.store((tail + 1) & kMask, std::memory_order_release);
        return true;
    }

    /** @brief True when the ring appears empty. Only reliable on the consumer side. */
    [[nodiscard]] bool empty() const noexcept
    {
        return m_tail.load(std::memory_order_acquire) == m_head.load(std::memory_order_acquire);
    }

   private:
    std::array<T, N> m_ring{};

    alignas(64) std::atomic<std::size_t> m_head{0};  // written by producer
    alignas(64) std::atomic<std::size_t> m_tail{0};  // written by consumer
};

}  // namespace Core