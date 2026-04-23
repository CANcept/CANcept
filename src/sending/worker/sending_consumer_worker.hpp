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

#include "sending/worker/scheduled_item_queue.hpp"

namespace Sending {

/**
 * @class SendingConsumerWorker
 * @brief Consumes ScheduledItems from the shared queue and fires each item's
 * onSend callback at the precise scheduledAt deadline.
 */
class SendingConsumerWorker final : public QThread
{
    Q_OBJECT

   public:
    explicit SendingConsumerWorker(ScheduledItemQueue& queue, QObject* parent = nullptr);
    ~SendingConsumerWorker() override;

    /**
     * @brief Starts the consumer loop.
     */
    void startConsuming();

    /**
     * @brief Stops the consumer loop gracefully.
     * Interrupts any blocking pop() before waiting for the thread.
     */
    void stopConsuming();

    /**
     * @brief Returns true if the consumer is currently running.
     */
    [[nodiscard]] auto isConsuming() const -> bool
    {
        return m_isActive.load();
    }

   signals:
    void consumingStarted();
    void consumingStopped();

    /**
     * @brief Emitted if an exception is caught while executing an item's onSend.
     */
    void errorOccurred(const QString& error);

   protected:
    void run() override;

   private:
    ScheduledItemQueue& m_queue;

    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
};

}  // namespace Sending