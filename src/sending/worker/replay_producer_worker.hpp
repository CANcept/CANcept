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
    /**
     * @brief Creates the replay producer worker.
     * @param queue Shared scheduled-item queue consumed by the sending consumer worker.
     * @param broker Event broker used to publish encoded RAW CAN frames.
     * @param parent Optional QObject parent.
     */
    explicit ReplayProducerWorker(ScheduledItemQueue& queue, Core::IEventBroker& broker,
                                  QObject* parent = nullptr);
    ~ReplayProducerWorker() override;

    /**
     * @brief Starts a new replay run.
     *
     * Stores the frame snapshot and speed factor, resets replay control flags,
     * bumps the replay token, and starts the worker thread.
     *
     * @param frames Replay frames to schedule.
     * @param speedFactor Playback speed multiplier (clamped internally).
     */
    void startReplay(const QList<Core::ReplayFrame>& frames, double speedFactor);

    /** @brief Requests replay pause. Already queued items may still be pending. */
    void pauseReplay();

    /** @brief Resumes a previously paused replay run. */
    void resumeReplay();

    /** @brief Stops replay production and invalidates stale queued replay items via token bump. */
    void stopReplay();

    /** @brief Indicates whether the replay worker currently has an active run. */
    [[nodiscard]] auto isRunningReplay() const -> bool
    {
        return m_isActive.load();
    }

    /** @brief Indicates whether the current replay run is paused. */
    [[nodiscard]] auto isPausedReplay() const -> bool
    {
        return m_isPaused.load();
    }

   signals:
    /** @brief Emitted once a replay run starts processing. */
    void replayStarted();

    /** @brief Emitted when replay is paused. */
    void replayPaused();

    /** @brief Emitted when replay is resumed after pause. */
    void replayResumed();

    /** @brief Emitted when the replay thread leaves its run loop (normal stop or error). */
    void replayStopped();

    /** @brief Emitted after the last frame of the active replay run was sent. */
    void replayFinished();

    /**
     * @brief Emitted with replay progress based on actually sent frames.
     * @param currentFrame Number of frames sent so far.
     * @param totalFrames Total number of frames in the run.
     */
    void progressUpdated(int currentFrame, int totalFrames);

    /**
     * @brief Emitted when replay processing fails.
     * @param error Human-readable error message.
     */
    void errorOccurred(const QString& error);

   protected:
    /** @brief Replay worker main loop. Schedules frames and pushes items to the queue. */
    void run() override;

   private:
    /** Shared queue between replay producer and sending consumer. */
    ScheduledItemQueue& m_queue;
    /** Event broker used for publishing send events from queued callbacks. */
    Core::IEventBroker& m_broker;

    /** Mutex/condition pair for pausing/resuming and state synchronization. */
    mutable std::mutex m_stateMutex;
    std::condition_variable m_stateCv;

    /** Snapshot of frames and speed used by the active run. */
    QList<Core::ReplayFrame> m_frames;
    double m_speedFactor = 1.0;

    /** Replay control flags shared across UI-thread calls and worker thread. */
    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<bool> m_isPaused{false};

    /**
     * @brief Monotonic run token used to invalidate stale queued replay items after stop/restart.
     */
    std::atomic<uint64_t> m_replayRunToken{0};
};

}  // namespace Sending
