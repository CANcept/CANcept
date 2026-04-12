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

#include "repeated_sending_worker.hpp"

#include <QThread>
#include <chrono>
#include <thread>

#include "sending/constants.hpp"

namespace Sending {

RepeatedSendingWorker::RepeatedSendingWorker(QObject* parent) : QThread(parent)
{
    setObjectName(Constants::REPEATED_SENDING_THREAD_NAME);
}

RepeatedSendingWorker::~RepeatedSendingWorker()
{
    stopSending();
}

void RepeatedSendingWorker::startSending(SendCallback callback, const int intervalUs)
{
    if (m_isActive.load())
    {
        emit errorOccurred(Constants::ERR_WORKER_ALREADY_RUNNING);
        return;
    }

    if (!callback)
    {
        emit errorOccurred(Constants::ERR_INVALID_CALLBACK);
        return;
    }

    if (intervalUs <= 0)
    {
        emit errorOccurred(Constants::ERR_INVALID_INTERVAL);
        return;
    }

    {
        std::lock_guard lock(m_callbackMutex);
        m_callback = std::move(callback);
    }

    m_intervalUs.store(intervalUs);
    m_shouldStop.store(false);
    m_isActive.store(true);

    start(QThread::TimeCriticalPriority);
}

void RepeatedSendingWorker::stopSending()
{
    if (!m_isActive.load())
    {
        return;
    }

    // Signal the worker to stop
    m_shouldStop.store(true);

    // Wait for the thread to finish
    if (!wait(Constants::THREAD_TERMINATION_WAIT_MS))
    {
        // If it doesn't Force termination
        terminate();
        wait();
    }

    m_isActive.store(false);

    {
        std::lock_guard lock(m_callbackMutex);
        m_callback = nullptr;
    }
}

void RepeatedSendingWorker::updateInterval(const int intervalUs)
{
    if (intervalUs > 0)
    {
        m_intervalUs.store(intervalUs);
        m_resetGuard.store(true);
    }
}

void RepeatedSendingWorker::run()
{
    using Clock = std::chrono::high_resolution_clock;
    using Ns = std::chrono::nanoseconds;
    auto guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);

    emit sendingStarted();

    // Snapshot the callback once
    SendCallback localCallback;
    {
        std::lock_guard lock(m_callbackMutex);
        localCallback = m_callback;
    }

    // Anchor the tick timeline, also helpful for variable interval later
    auto nextDeadline = Clock::now();

    while (!m_shouldStop.load())
    {
        const long long intervalNs = static_cast<long long>(m_intervalUs.load()) * 1'000LL;
        nextDeadline += Ns(intervalNs);

        if (localCallback)
        {
            try
            {
                localCallback();
            } catch (const std::exception& e)
            {
                emit errorOccurred(
                    QString(Constants::ERR_CALLBACK_EXCEPTION_TEMPLATE).arg(e.what()));
            } catch (...)
            {
                emit errorOccurred(Constants::ERR_UNKNOWN_CALLBACK_ERROR);
            }
        }

        if (m_resetGuard.exchange(false))
        {
            // Interval changed — resync the timeline to prevent an immediate burst.
            guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);
            nextDeadline = Clock::now() + Ns(intervalNs);
        }

        // If the callback overran the deadline, resync
        if (Clock::now() >= nextDeadline)
        {
            nextDeadline = Clock::now();
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
            guardNs = std::max(static_cast<double>(Constants::MIN_SLEEP_GUARD_NS),
                               std::max(guardNs * 0.99, newEstimate));
        }

        while (Clock::now() < nextDeadline && !m_shouldStop.load())
        {
        }
    }

    emit sendingStopped();
}

}  // namespace Sending
