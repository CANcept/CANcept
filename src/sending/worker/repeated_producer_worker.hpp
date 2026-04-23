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

#include <QString>
#include <QThread>
#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

#include "sending/worker/scheduled_item_queue.hpp"

namespace Sending {

/**
 * @class RepeatedProducerWorker
 * @brief Runs a create callback on a dedicated thread at a fixed interval,
 *        pushing produced ScheduledItems into the shared queue for the consumer.
 */
class RepeatedProducerWorker final : public QThread
{
    Q_OBJECT

    using Clock = std::chrono::high_resolution_clock;
    using CreateCallback = std::function<std::vector<ScheduledItem>(Clock::time_point)>;

   public:
    explicit RepeatedProducerWorker(ScheduledItemQueue& queue, QObject* parent = nullptr);
    ~RepeatedProducerWorker() override;

    /**
     * @brief Starts the producer loop with the given callback and interval.
     * @param callback Invoked each iteration to produce the next batch.
     * @param intervalUs Interval in microseconds between batches.
     */
    void startCreating(CreateCallback callback, int intervalUs);

    /**
     * @brief Stops the producer loop gracefully.
     */
    void stopCreating();

    /**
     * @brief Informs whether the producer currently produces.
     * @return if it produces.
     */
    bool isRunning() const;

    /**
     * @brief Updates the interval while running.
     * @param intervalUs New interval in microseconds.
     */
    void updateInterval(int intervalUs);

   signals:
    /**
     * @brief Emitted if an exception is caught during item creation.
     */
    void errorOccurred(const QString& error);

   protected:
    void run() override;

   private:
    mutable std::mutex m_callbackMutex;
    CreateCallback m_callback;

    ScheduledItemQueue& m_queue;

    std::atomic<int> m_intervalUs{1000};
    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<bool> m_resetGuard{false};
};

}  // namespace Sending