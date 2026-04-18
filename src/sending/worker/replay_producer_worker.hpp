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

#include <QThread>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "core/dto/replay_dto.hpp"
#include "core/interface/i_event_broker.hpp"
#include "sending/worker/scheduled_item_queue.hpp"

namespace Sending {

/**
 * @class ReplayProducerWorker
 * @brief Produces replay frames with recorded timing and speed scaling.
 *
 * The worker waits according to relative log timestamps (normalized to the
 * first frame), then enqueues ScheduledItems into the shared queue.
 */
class ReplayProducerWorker final : public QThread
{
    Q_OBJECT

   public:
    explicit ReplayProducerWorker(ScheduledItemQueue& queue, Core::IEventBroker& broker,
                                  QObject* parent = nullptr);
    ~ReplayProducerWorker() override;

    void startReplay(const QList<Core::ReplayFrame>& frames, double speedFactor);
    void pauseReplay();
    void resumeReplay();
    void stopReplay();

    [[nodiscard]] auto isRunningReplay() const -> bool
    {
        return m_isActive.load();
    }

    [[nodiscard]] auto isPausedReplay() const -> bool
    {
        return m_isPaused.load();
    }

   signals:
    void replayStarted();
    void replayPaused();
    void replayResumed();
    void replayStopped();
    void replayFinished();
    void progressUpdated(int currentFrame, int totalFrames);
    void errorOccurred(const QString& error);

   protected:
    void run() override;

   private:
    ScheduledItemQueue& m_queue;
    Core::IEventBroker& m_broker;

    mutable std::mutex m_stateMutex;
    std::condition_variable m_stateCv;

    QList<Core::ReplayFrame> m_frames;
    double m_speedFactor = 1.0;

    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<bool> m_isPaused{false};
    std::atomic<uint64_t> m_replayRunToken{0};
};

}  // namespace Sending

