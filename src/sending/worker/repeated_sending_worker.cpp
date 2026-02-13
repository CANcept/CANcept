#include "repeated_sending_worker.hpp"

#include <QThread>
#include <chrono>

#include "sending/constants.hpp"

namespace Sending {

RepeatedSendingWorker::RepeatedSendingWorker(QObject* parent) : QThread(parent) {}

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

    start();
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
    }
}

void RepeatedSendingWorker::run()
{
    emit sendingStarted();

    while (!m_shouldStop.load())
    {
        // Execute the send callback
        {
            std::lock_guard lock(m_callbackMutex);
            if (m_callback)
            {
                try
                {
                    m_callback();
                } catch (const std::exception& e)
                {
                    emit errorOccurred(QString(Constants::ERR_CALLBACK_EXCEPTION_TEMPLATE).arg(e.what()));
                } catch (...)
                {
                    emit errorOccurred(Constants::ERR_UNKNOWN_CALLBACK_ERROR);
                }
            }
        }

        // Sleep for the specified interval, also checks periodicallly for stop signal
        const int intervalMs = m_intervalMs.load();
        int remainingMs = intervalMs;

        while (remainingMs > 0 && !m_shouldStop.load())
        {
            const int sleepMs = std::min(remainingMs, Constants::POLLING_CHECK_INTERVAL_MS);
            msleep(sleepMs);
            remainingMs -= sleepMs;
        }
    }

    emit sendingStopped();
}

}  // namespace Sending
