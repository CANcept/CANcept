#pragma once

#include <QThread>
#include <atomic>
#include <functional>
#include <mutex>

namespace Sending {

/**
 * @class RepeatedSendingWorker
 * @brief Worker thread for cyclic CAN message transmission.
 *
 * This worker runs in a separate thread and repeatedly executes a send callback
 * at a specified interval, given during start.
 */
class RepeatedSendingWorker final : public QThread
{
    Q_OBJECT

   public:
    using SendCallback = std::function<void()>;

    explicit RepeatedSendingWorker(QObject* parent = nullptr);
    ~RepeatedSendingWorker() override;

    /**
     * @brief Starts the worker with the given callback and interval.
     * @param callback Function to call at each interval
     * @param intervalMs Interval in milliseconds between sends
     */
    void startSending(SendCallback callback, int intervalMs);

    /**
     * @brief Stops the worker gracefully.
     */
    void stopSending();

    /**
     * @brief Updates the send interval while running.
     * @param intervalMs New interval in milliseconds
     */
    void updateInterval(int intervalMs);

    /**
     * @brief Checks if the worker is currently sending.
     * @return true if actively sending, false otherwise
     */
    [[nodiscard]] auto isSending() const -> bool
    {
        return m_isActive.load();
    }

   signals:
    /**
     * @brief Emitted when the worker starts sending.
     */
    void sendingStarted();

    /**
     * @brief Emitted when the worker stops sending.
     */
    void sendingStopped();

    /**
     * @brief Emitted if an error occurs during sending.
     */
    void errorOccurred(const QString& error);

   protected:
    void run() override;

   private:
    std::atomic<bool> m_isActive{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<int> m_intervalMs{Constants::DEFAULT_CYCLE_INTERVAL_MS};

    mutable std::mutex m_callbackMutex;
    SendCallback m_callback;
};

}  // namespace Sending
