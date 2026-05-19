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
#include <exception>
#include <thread>

#include "core/event/can_event.hpp"
#include "sending/constants.hpp"
#include "sending/sending_functions.hpp"

namespace Sending {

ReplayProducerWorker::ReplayProducerWorker(ScheduledItemQueue& queue, Core::IEventBroker& broker,
                                           QObject* parent)
    : QThread(parent), m_queue(queue), m_broker(broker)
{
    setObjectName(Constants::REPLAY_PRODUCER_WORKER_THREAD_NAME);
}

ReplayProducerWorker::~ReplayProducerWorker()
{
    stopReplay();
}

void ReplayProducerWorker::startReplay(ReaderFactory factory, const uint64_t frameCount,
                                       const double speedFactor,
                                       std::shared_ptr<Core::IFaultHandler> faultHandler)
{
    if (m_isActive.load())
    {
        stopReplay();
    }

    if (!factory)
    {
        return;
    }

    {
        std::lock_guard lock(m_stateMutex);
        m_factory = std::move(factory);
        m_totalFrames = frameCount;
        m_speedFactor = std::clamp(speedFactor, 0.1, 8.0);
        m_faultHandler = std::move(faultHandler);
    }

    m_scheduledCount.store(0);
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
        ReaderFactory localFactory;
        uint64_t localTotal;
        double localSpeed;
        std::shared_ptr<Core::IFaultHandler> localFaultHandler;
        {
            std::scoped_lock lock(m_stateMutex);
            localFactory = m_factory;
            localTotal = m_totalFrames;
            localSpeed = m_speedFactor;
            localFaultHandler = m_faultHandler;
        }

        using Ns = std::chrono::nanoseconds;
        constexpr auto LOOKAHEAD = std::chrono::microseconds(Constants::REPLAY_LOOKAHEAD_US);

        auto guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);

        const auto reader = localFactory();
        if (!reader)
        {
            m_isActive.store(false);
            emit errorOccurred("Replay: failed to open reader");
            emit replayStopped();
            return;
        }

        emit replayStarted();

        if (reader->eof())
        {
            m_isActive.store(false);
            emit replayStopped();
            return;
        }

        const bool isDbc = (reader->fileType() == Core::CanFileType::Dbc);
        auto readNext = [&](Core::RawCanMessage& out) -> bool {
            if (!isDbc)
            {
                return reader->read(out);
            }
            Core::DbcCanMessage dbcMsg;
            if (!reader->read(dbcMsg)) return false;
            if (localFaultHandler)
            {
                localFaultHandler->inject(dbcMsg);
            }
            out = {};
            out.receiveTime = dbcMsg.receiveTime;
            m_broker.publish(Core::EncodeCanMessageDbcEvent(dbcMsg, out));
            return true;
        };

        Core::RawCanMessage firstMsg{};
        if (!readNext(firstMsg))
        {
            m_isActive.store(false);
            emit replayStopped();
            return;
        }

        const auto firstTimestamp = firstMsg.receiveTime;
        auto replayStart = Clock::now();

        auto scheduleAndPush = [&](const Core::RawCanMessage& msg) -> void {
            const auto relativeNs = msg.receiveTime - firstTimestamp;
            const auto scaledNs =
                static_cast<long long>(static_cast<double>(relativeNs.count()) / localSpeed);
            auto scheduledAt = replayStart + Ns(std::max<long long>(0LL, scaledNs));

            while (!m_shouldStop.load())
            {
                if (m_isPaused.load())
                {
                    const auto pauseBegin = Clock::now();
                    {
                        std::unique_lock lock(m_stateMutex);
                        m_stateCv.wait(
                            lock, [this] { return m_shouldStop.load() || !m_isPaused.load(); });
                    }
                    replayStart += Clock::now() - pauseBegin;
                    scheduledAt = replayStart + Ns(std::max<long long>(0LL, scaledNs));
                    continue;
                }

                const auto windowOpen = scheduledAt - LOOKAHEAD;
                const auto now = Clock::now();
                const long long leftNs = std::chrono::duration_cast<Ns>(windowOpen - now).count();

                if (leftNs <= 0)
                {
                    break;
                }

                if (leftNs > static_cast<long long>(guardNs))
                {
                    // Sleep most of the remaining wait and cap at 50 ms for pause/stop
                    const auto sleepUntil =
                        std::min(windowOpen - Ns(static_cast<long long>(guardNs)),
                                 now + std::chrono::milliseconds(50));
                    std::this_thread::sleep_until(sleepUntil);

                    const long long overshootNs =
                        std::chrono::duration_cast<Ns>(Clock::now() - sleepUntil).count();
                    const double newEst =
                        Constants::SLEEP_GUARD_ALPHA * static_cast<double>(overshootNs) +
                        (1.0 - Constants::SLEEP_GUARD_ALPHA) * guardNs;
                    guardNs = std::max({static_cast<double>(Constants::MIN_SLEEP_GUARD_NS),
                                        guardNs * 0.99, newEst});
                }
            }

            if (m_shouldStop.load())
            {
                return;
            }

            Core::RawCanMessage mutMsg = msg;
            auto finalScheduledAt = scheduledAt;
            if (localFaultHandler)
            {
                localFaultHandler->inject(mutMsg.messageId, mutMsg.dlc, mutMsg.data);
                const auto [drop, delay] = localFaultHandler->evaluate();
                if (drop)
                {
                    return;
                }
                if (delay.count() > 0)
                {
                    finalScheduledAt += delay;
                }
            }

            auto context = std::make_shared<RawSendContext>(
                RawSendContext{.broker = &m_broker, .message = mutMsg});

            ScheduledItem item{.scheduledAt = finalScheduledAt,
                               .onSend = &rawSendImpl,
                               .context = std::move(context)};
            while (!m_shouldStop.load() && !m_queue.tryPush(item))
            {
            }
        };

        scheduleAndPush(firstMsg);
        m_scheduledCount.fetch_add(1, std::memory_order_relaxed);

        while (!reader->eof() && !m_shouldStop.load())
        {
            Core::RawCanMessage msg{};
            if (!readNext(msg))
            {
                break;
            }
            scheduleAndPush(msg);
            m_scheduledCount.fetch_add(1, std::memory_order_relaxed);
        }

        m_isActive.store(false);
        if (!m_shouldStop.load())
        {
            emit replayFinished();
        }
        emit replayStopped();
    } catch (const std::exception& ex)
    {
        m_isActive.store(false);
        emit errorOccurred(QString("Replay worker error: %1").arg(ex.what()));
        emit replayStopped();
    } catch (...)
    {
        m_isActive.store(false);
        emit errorOccurred("Replay worker: unknown error");
        emit replayStopped();
    }
}

}  // namespace Sending