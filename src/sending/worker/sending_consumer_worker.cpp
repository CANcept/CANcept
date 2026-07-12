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

#include "sending_consumer_worker.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

#include "sending/constants.hpp"

namespace Sending {

SendingConsumerWorker::SendingConsumerWorker(ScheduledItemQueue& queue, QObject* parent)
    : QThread(parent), m_queue(queue)
{
    setObjectName(Constants::SENDING_CONSUMER_WORKER_THREAD_NAME);
}

SendingConsumerWorker::~SendingConsumerWorker()
{
    stopConsuming();
}

void SendingConsumerWorker::startConsuming()
{
    if (m_isActive.load())
    {
        return;
    }

    m_shouldStop.store(false);
    m_isActive.store(true);

    start(QThread::TimeCriticalPriority);
}

void SendingConsumerWorker::stopConsuming()
{
    if (!m_isActive.load())
    {
        return;
    }

    m_shouldStop.store(true);
    m_queue.interrupt();

    if (!wait(Constants::THREAD_TERMINATION_WAIT_MS))
    {
        terminate();
        wait();
    }

    m_isActive.store(false);
    emit consumingStopped();
}

void SendingConsumerWorker::run()
{
    using Ns = std::chrono::nanoseconds;
    auto guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);

    emit consumingStarted();

    while (!m_shouldStop.load())
    {
        auto optItem = m_queue.pop();

        if (!optItem)
        {
            continue;
        }

        auto item = std::move(*optItem);

        while (Clock::now() < item.scheduledAt)
        {
            if (m_shouldStop.load())
            {
                break;
            }

            const long long remainingNs =
                std::chrono::duration_cast<Ns>(item.scheduledAt - Clock::now()).count();

            if (remainingNs > static_cast<long long>(guardNs))
            {
                if (const auto earliest = m_queue.peekFront();
                    earliest && *earliest < item.scheduledAt)
                {
                    if (m_queue.tryPush(item))
                    {
                        auto requeued = m_queue.pop();
                        if (!requeued)
                        {
                            break;
                        }
                        item = std::move(*requeued);
                        continue;
                    }
                }

                const auto fullSleepTarget = item.scheduledAt - Ns(static_cast<long long>(guardNs));
                const auto sleepTarget =
                    std::min(fullSleepTarget, Clock::now() + Ns(Constants::MIN_SLEEP_GUARD_NS));

                std::this_thread::sleep_until(sleepTarget);

                const long long overshootNs =
                    std::chrono::duration_cast<Ns>(Clock::now() - sleepTarget).count();
                const double newEstimate =
                    Constants::SLEEP_GUARD_ALPHA * static_cast<double>(overshootNs) +
                    (1.0 - Constants::SLEEP_GUARD_ALPHA) * guardNs;
                guardNs = std::max({static_cast<double>(Constants::MIN_SLEEP_GUARD_NS),
                                    guardNs * 0.99, newEstimate});
            } else
            {
                std::this_thread::yield();
            }
        }

        if (m_shouldStop.load())
        {
            break;
        }

        if (!item.onSend)
        {
            continue;
        }

        try
        {
            item.onSend(item.context.get());
        } catch (const std::exception& e)
        {
            emit errorOccurred(QString(Constants::ERR_CALLBACK_EXCEPTION_TEMPLATE).arg(e.what()));
        } catch (...)
        {
            emit errorOccurred(Constants::ERR_UNKNOWN_CALLBACK_ERROR);
        }
    }
}

}  // namespace Sending