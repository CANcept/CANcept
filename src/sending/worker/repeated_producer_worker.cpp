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

#include "repeated_producer_worker.hpp"

#include <chrono>
#include <thread>

#include "sending/constants.hpp"

namespace Sending {

RepeatedProducerWorker::RepeatedProducerWorker(ScheduledItemQueue& queue, QObject* parent)
    : QThread(parent), m_queue(queue)
{
    setObjectName(Constants::REPEATED_PRODUCER_WORKER_THREAD_NAME);
}

RepeatedProducerWorker::~RepeatedProducerWorker()
{
    stopCreating();
}

void RepeatedProducerWorker::startCreating(CreateCallback callback, const int intervalUs)
{
    if (m_isActive.load())
    {
        return;
    }

    if (!callback)
    {
        return;
    }

    if (intervalUs <= 0)
    {
        return;
    }

    {
        std::lock_guard lock(m_callbackMutex);
        m_callback = std::move(callback);
    }

    m_intervalUs.store(intervalUs);
    m_shouldStop.store(false);
    m_isActive.store(true);

    start(QThread::HighPriority);
}

void RepeatedProducerWorker::stopCreating()
{
    if (!m_isActive.load())
    {
        return;
    }

    m_shouldStop.store(true);

    if (!wait(Constants::THREAD_TERMINATION_WAIT_MS))
    {
        terminate();
        wait();
    }

    m_isActive.store(false);

    {
        std::lock_guard lock(m_callbackMutex);
        m_callback = nullptr;
    }
}

void RepeatedProducerWorker::updateInterval(const int intervalUs)
{
    if (intervalUs > 0)
    {
        m_intervalUs.store(intervalUs);
        m_resetGuard.store(true);
    }
}

bool RepeatedProducerWorker::isRunning() const
{
    return m_isActive.load();
}

void RepeatedProducerWorker::run()
{
    using Ns = std::chrono::nanoseconds;
    auto guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);

    CreateCallback localCallback;
    {
        std::lock_guard lock(m_callbackMutex);
        localCallback = m_callback;
    }

    auto nextDeadline = Clock::now();

    while (!m_shouldStop.load())
    {
        const long long intervalNs = static_cast<long long>(m_intervalUs.load()) * 1'000LL;
        nextDeadline += Ns(intervalNs);

        try
        {
            for (auto items = localCallback(nextDeadline); auto& item : items)
            {
                m_queue.push(std::move(item));
            }
        } catch (const std::exception& e)
        {
            emit errorOccurred(QString(Constants::ERR_CREATE_EXCEPTION_TEMPLATE).arg(e.what()));
        } catch (...)
        {
            emit errorOccurred(Constants::ERR_UNKNOWN_CREATE_ERROR);
        }

        if (m_resetGuard.exchange(false))
        {
            nextDeadline = Clock::now() + Ns(intervalNs);
            guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);
        }

        if (Clock::now() >= nextDeadline)
        {
            continue;
        }

        const long long remainingNs =
            std::chrono::duration_cast<Ns>(nextDeadline - Clock::now()).count();

        if (remainingNs > static_cast<long long>(guardNs))
        {
            const auto sleepTarget = nextDeadline - Ns(static_cast<long long>(guardNs));
            std::this_thread::sleep_until(sleepTarget);

            const long long overshootNs =
                std::chrono::duration_cast<Ns>(Clock::now() - sleepTarget).count();
            const double newEstimate =
                Constants::SLEEP_GUARD_ALPHA * static_cast<double>(overshootNs) +
                (1.0 - Constants::SLEEP_GUARD_ALPHA) * guardNs;
            guardNs = std::max(
                {static_cast<double>(Constants::MIN_SLEEP_GUARD_NS), guardNs * 0.99, newEstimate});
        }

        while (Clock::now() < nextDeadline && !m_shouldStop.load())
        {
        }
    }
}

}  // namespace Sending