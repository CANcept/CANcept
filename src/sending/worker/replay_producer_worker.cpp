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

#include "replay_producer_worker.hpp"

#include <algorithm>
#include <cmath>
#include <exception>

#include "sending/constants.hpp"
#include "sending/sending_functions.hpp"

namespace Sending {

ReplayProducerWorker::ReplayProducerWorker(ScheduledItemQueue& queue, Core::IEventBroker& broker,
                                           QObject* parent)
    : QThread(parent), m_queue(queue), m_broker(broker)
{
    setObjectName("ReplayProducerWorker");
}

ReplayProducerWorker::~ReplayProducerWorker()
{
    stopReplay();
}

void ReplayProducerWorker::startReplay(const QList<Core::ReplayFrame>& frames,
                                       const double speedFactor)
{
    if (m_isActive.load())
    {
        stopReplay();
    }

    if (frames.isEmpty())
    {
        return;
    }

    {
        std::lock_guard lock(m_stateMutex);
        m_frames = frames;
        m_speedFactor = std::clamp(speedFactor, 0.1, 8.0);
    }

    m_replayRunToken.fetch_add(1);
    m_shouldStop.store(false);
    m_isPaused.store(false);
    m_isActive.store(true);

    start(QThread::HighPriority);
}

void ReplayProducerWorker::pauseReplay()
{
    if (!m_isActive.load() || m_isPaused.load())
    {
        return;
    }

    m_isPaused.store(true);
    emit replayPaused();
}

void ReplayProducerWorker::resumeReplay()
{
    if (!m_isActive.load() || !m_isPaused.load())
    {
        return;
    }

    m_isPaused.store(false);
    m_stateCv.notify_all();
    emit replayResumed();
}

void ReplayProducerWorker::stopReplay()
{
    if (!m_isActive.load())
    {
        return;
    }

    m_replayRunToken.fetch_add(1);
    m_shouldStop.store(true);
    m_isPaused.store(false);
    m_stateCv.notify_all();

    if (!wait(Constants::THREAD_TERMINATION_WAIT_MS))
    {
        terminate();
        wait();
    }

    m_isActive.store(false);
}

void ReplayProducerWorker::run()
{
    try
    {
        QList<Core::ReplayFrame> localFrames;
        double localSpeed = 1.0;
        {
            std::lock_guard lock(m_stateMutex);
            localFrames = m_frames;
            localSpeed = m_speedFactor;
        }

        const uint64_t runToken = m_replayRunToken.load();
        const int totalFrames = localFrames.size();
        const auto sentCounter = std::make_shared<std::atomic<int>>(0);
        constexpr auto LOOKAHEAD_MS = std::chrono::milliseconds(200);

        emit replayStarted();

        if (totalFrames == 0)
        {
            m_isActive.store(false);
            emit replayStopped();
            return;
        }

        const auto replayStartNow = Clock::now();
        const auto firstTimestamp = localFrames.first().receiveTime;

        for (int i = 0; i < totalFrames; ++i)
        {
            if (m_shouldStop.load())
            {
                break;
            }

            if (m_isPaused.load())
            {
                std::unique_lock lock(m_stateMutex);
                m_stateCv.wait(lock,
                               [this]() { return m_shouldStop.load() || !m_isPaused.load(); });
                if (m_shouldStop.load())
                {
                    break;
                }
            }

            const auto relativeFromStart = std::max<std::chrono::milliseconds>(
                std::chrono::milliseconds(0), localFrames.at(i).receiveTime - firstTimestamp);

            const auto scaledMs = static_cast<long long>(
                std::llround(static_cast<double>(relativeFromStart.count()) / localSpeed));
            const auto scheduledAt =
                replayStartNow + std::chrono::milliseconds(std::max<long long>(0, scaledMs));

            // Wait until frame is within lookahead window
            auto now = Clock::now();
            auto timeUntilScheduled = scheduledAt - now;
            while (timeUntilScheduled > LOOKAHEAD_MS)
            {
                if (m_shouldStop.load())
                {
                    break;
                }

                std::unique_lock lock(m_stateMutex);
                const auto waitTime =
                    std::min(std::chrono::duration_cast<std::chrono::milliseconds>(
                                 timeUntilScheduled - LOOKAHEAD_MS),
                             std::chrono::milliseconds(100));
                m_stateCv.wait_for(lock, waitTime,
                                   [this]() { return m_shouldStop.load() || m_isPaused.load(); });
                if (m_shouldStop.load())
                {
                    break;
                }

                if (m_isPaused.load())
                {
                    m_stateCv.wait(lock,
                                   [this]() { return m_shouldStop.load() || !m_isPaused.load(); });
                    if (m_shouldStop.load())
                    {
                        break;
                    }
                }

                now = Clock::now();
                timeUntilScheduled = scheduledAt - now;
            }

            if (m_shouldStop.load())
            {
                break;
            }

            const auto& frame = localFrames.at(i);
            Core::RawCanMessage msg{};
            msg.messageId = frame.messageId;
            msg.dlc = frame.dlc;

            for (size_t b = 0; b < frame.data.size(); ++b)
            {
                msg.data[b] = static_cast<char>(frame.data[b]);
            }

            auto context = std::make_shared<RawSendContext>(
                RawSendContext{.broker = &m_broker,
                               .message = msg,
                               .replayToken = runToken,
                               .activeReplayToken = &m_replayRunToken,
                               .onSent = [this, sentCounter, totalFrames, runToken]() {
                                   if (runToken != m_replayRunToken.load())
                                   {
                                       return;
                                   }
                                   const int sent = sentCounter->fetch_add(1) + 1;
                                   emit progressUpdated(sent, totalFrames);
                                   if (sent == totalFrames && !m_shouldStop.load())
                                   {
                                       emit replayFinished();
                                   }
                               }});
            m_queue.push(ScheduledItem{
                .scheduledAt = scheduledAt, .onSend = &rawSendImpl, .context = std::move(context)});
        }

        m_isActive.store(false);
        emit replayStopped();
    } catch (const std::exception& ex)
    {
        m_isActive.store(false);
        emit errorOccurred(QString("Replay worker error: %1").arg(ex.what()));
        emit replayStopped();
    } catch (...)
    {
        m_isActive.store(false);
        emit errorOccurred("Replay worker unknown error");
        emit replayStopped();
    }
}

}  // namespace Sending
