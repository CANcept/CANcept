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
#include <functional>
#include <memory>
#include <mutex>

#include "core/interface/i_can_reader.hpp"
#include "core/interface/i_event_broker.hpp"
#include "core/interface/i_fault_handler.hpp"
#include "sending/worker/scheduled_item_queue.hpp"

namespace Sending {

/**
 * @class ReplayProducerWorker
 * @brief Streams CAN frames from any ICanReader source and schedules them with recorded timing.
 */
class ReplayProducerWorker final : public QThread
{
    Q_OBJECT

   public:
    /**
     * @brief Factory callable that constructs and opens a reader.
     */
    using ReaderFactory = std::function<std::unique_ptr<Core::ICanReader>()>;

    /**
     * @brief Creates the replay producer worker.
     * @param queue Shared scheduled-item queue consumed by SendingConsumerWorker.
     * @param broker Event broker used to publish encoded RAW CAN frames.
     * @param parent Optional QObject parent.
     */
    explicit ReplayProducerWorker(ScheduledItemQueue& queue, Core::IEventBroker& broker,
                                  QObject* parent = nullptr);
    ~ReplayProducerWorker() override;

    /**
     * @brief Starts a new replay run.
     *
     * @param factory   Factory that opens the appropriate ICanReader on the worker thread.
     * @param frameCount Total record count used for progress reporting.
     * @param speedFactor Playback speed multiplier (clamped to [0.1, 8.0]).
     */
    void startReplay(ReaderFactory factory, uint64_t frameCount, double speedFactor,
                     std::shared_ptr<Core::IFaultHandler> faultHandler = nullptr);

    /** @brief Requests replay pause. Already queued items may still be pending. */
    void pauseReplay();

    /** @brief Resumes a previously paused replay run. */
    void resumeReplay();

    /** @brief Stops replay production and invalidates stale queued items via token bump. */
    void stopReplay();

    /** @brief Returns true while a replay run is active. */
    [[nodiscard]] auto isRunningReplay() const -> bool
    {
        return m_isActive.load();
    }

    /** @brief Returns true while the current replay run is paused. */
    [[nodiscard]] auto isPausedReplay() const -> bool
    {
        return m_isPaused.load();
    }

    /** @brief Number of frames scheduled so far; safe to read from any thread. */
    [[nodiscard]] auto scheduledFrameCount() const -> int
    {
        return m_scheduledCount.load(std::memory_order_relaxed);
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

    /** @brief Emitted after the last frame of the active run was scheduled. */
    void replayFinished();

    /**
     * @brief Emitted when replay processing fails.
     * @param error Human-readable error message.
     */
    void errorOccurred(const QString& error);

   protected:
    void run() override;

   private:
    ScheduledItemQueue& m_queue;
    Core::IEventBroker& m_broker;

    mutable std::mutex m_stateMutex;
    std::condition_variable m_stateCv;

    ReaderFactory m_factory;
    uint64_t m_totalFrames = 0;
    double m_speedFactor = 1.0;
    std::shared_ptr<Core::IFaultHandler> m_faultHandler;

    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<bool> m_isPaused{false};

    /** @brief Frames scheduled so far in the current run; polled by the UI timer. */
    std::atomic<int> m_scheduledCount{0};
};

}  // namespace Sending