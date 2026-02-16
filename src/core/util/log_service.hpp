#pragma once

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Core {

/**
 * @enum LogContext
 * @brief Categorizes logging output. CanLogging is session-dependent,
 * while Debug is a continuous system-wide diagnostic stream.
 */
enum class LogContext { CanLogging, Debug };

/**
 * @class LogService
 * @brief A Singleton service managing asynchronous loggers and session-based file lifecycle.
 * * The LogService provides a global access point for logging.
 */
class LogService
{
   public:
    /**
     * @brief Accesses the global instance of the LogService.
     * @return LogService& The singleton instance.
     */
    static LogService& getInstance()
    {
        static LogService instance;
        return instance;
    }

    // Delete copy and assignment to enforce Singleton pattern
    LogService(const LogService&) = delete;
    LogService& operator=(const LogService&) = delete;

    /**
     * @brief Retrieves a logger for a specific context and session.
     * * If the logger for the given session ID and context does not exist,
     * it is created using the naming convention: "logs/session_{sessionId}_{context}.log".
     * * @param context The type of log (CanLogging or Debug).
     * @param sessionId The unique identifier for the logging session.
     * Defaults to "system" for Debug context.
     * @return std::shared_ptr<spdlog::logger> The asynchronous logger instance.
     */
    std::shared_ptr<spdlog::logger> getLogger(LogContext context,
                                              const std::string& sessionId = "system");

    /**
     * @brief Flushes all active loggers to disk.
     * Should be called during critical errors or application shutdown.
     */
    void flushAll();

    /**
     * @brief Closes a specific session logger and removes it from the registry.
     * This releases the file handle, allowing the file to be converted or moved.
     * @param context The context of the logger.
     * @param sessionId The session ID to close.
     */
    void closeLogger(LogContext context, const std::string& sessionId);

    static auto getLogFilePath(LogContext context, const std::string& sessionId) -> std::string;

   private:
    /**
     * @brief Private constructor to initialize the spdlog thread pool.
     */
    LogService();

    /**
     * @brief Private destructor to ensure clean shutdown of all loggers.
     */
    ~LogService();

    /**
     * @brief Helper to generate a unique key for the internal registry map.
     */
    auto createRegistryKey(LogContext context, const std::string& sessionId) -> std::string;

    std::mutex m_registryMutex;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> m_loggers;
};

}  // namespace Core