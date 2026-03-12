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

void RepeatedSendingWorker::startSending(SendCallback callback, const int intervalMs)
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

    if (intervalMs <= 0)
    {
        emit errorOccurred(Constants::ERR_INVALID_INTERVAL);
        return;
    }

    {
        std::lock_guard lock(m_callbackMutex);
        m_callback = std::move(callback);
    }

    m_intervalMs.store(intervalMs);
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

void RepeatedSendingWorker::updateInterval(const int intervalMs)
{
    if (intervalMs > 0)
    {
        m_intervalMs.store(intervalMs);
        m_resetGuard.store(true);
    }
}

void RepeatedSendingWorker::run()
{
    using Clock = std::chrono::high_resolution_clock;
    using Ns = std::chrono::nanoseconds;
    auto guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);

    emit sendingStarted();

    while (!m_shouldStop.load())
    {
        const auto deadline =
            Clock::now() + Ns(static_cast<long long>(m_intervalMs.load()) * 1'000'000LL);

        {
            std::lock_guard lock(m_callbackMutex);
            if (m_callback)
            {
                try
                {
                    m_callback();
                } catch (const std::exception& e)
                {
                    emit errorOccurred(
                        QString(Constants::ERR_CALLBACK_EXCEPTION_TEMPLATE).arg(e.what()));
                } catch (...)
                {
                    emit errorOccurred(Constants::ERR_UNKNOWN_CALLBACK_ERROR);
                }
            }
        }

        if (m_resetGuard.exchange(false))
        {
            guardNs = static_cast<double>(Constants::INITIAL_SLEEP_GUARD_NS);
        }

        const long long remainingNs =
            std::chrono::duration_cast<Ns>(deadline - Clock::now()).count();

        if (remainingNs > static_cast<long long>(guardNs))
        {
            const auto sleepTarget = deadline - Ns(static_cast<long long>(guardNs));
            std::this_thread::sleep_until(sleepTarget);
            const long long overshootNs =
                std::chrono::duration_cast<Ns>(Clock::now() - sleepTarget).count();

            const double newEstimate =
                Constants::SLEEP_GUARD_ALPHA * static_cast<double>(overshootNs) +
                (1.0 - Constants::SLEEP_GUARD_ALPHA) * guardNs;
            guardNs = std::max(static_cast<double>(Constants::MIN_SLEEP_GUARD_NS),
                               std::max(guardNs * 0.99, newEstimate));
        }

        while (Clock::now() < deadline && !m_shouldStop.load())
        {
        }
    }

    emit sendingStopped();
}

}  // namespace Sending
