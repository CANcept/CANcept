#pragma once

#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace Logging {

/**
 * @class Logger
 * @brief Wrapper for spdlog to provide centralized logging for the Logging module.
 *
 * This class provides a singleton interface to spdlog with both console and file outputs.
 * It supports multiple log levels and automatic log rotation.
 */
class Logger
{
   public:
    /**
     * @brief Get the singleton instance of the logger.
     * @return Reference to the logger instance.
     */
    static Logger& instance();

    /**
     * @brief Initialize the logger with file and console sinks.
     * @param logFilePath Path to the log file (default: "logs/logging_module.log").
     * @param maxFileSize Maximum size of each log file in bytes (default: 5MB).
     * @param maxFiles Maximum number of rotated log files to keep (default: 3).
     */
    void initialize(const std::string& logFilePath = "logs/logging_module.log",
                    size_t maxFileSize = 1024 * 1024 * 5, size_t maxFiles = 3);

    /**
     * @brief Set the logging level.
     * @param level spdlog::level::level_enum (trace, debug, info, warn, err, critical, off).
     */
    void setLevel(spdlog::level::level_enum level);

    /**
     * @brief Get the current logging level.
     * @return Current spdlog::level::level_enum.
     */
    spdlog::level::level_enum getLevel() const;

    /**
     * @brief Set the log message pattern/format.
     * @param pattern Format pattern string (e.g., "[%Y-%m-%d %H:%M:%S.%e] [%l] %v").
     */
    void setPattern(const std::string& pattern);

    /**
     * @brief Check if the logger has been initialized.
     * @return true if initialized, false otherwise.
     */
    bool isInitialized() const;

    // Logging methods
    template <typename... Args>
    void trace(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->trace(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->debug(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->info(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->warn(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->error(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(fmt::format_string<Args...> format, Args&&... args)
    {
        m_logger->critical(format, std::forward<Args>(args)...);
    }

    /**
     * @brief Flush all pending log messages to disk.
     */
    void flush();

    /**
     * @brief Enable or disable backtrace logging.
     * When enabled, recent log messages are stored in a ring buffer and dumped on demand.
     * @param numMessages Number of messages to store in the backtrace buffer.
     */
    void enableBacktrace(size_t numMessages);

    /**
     * @brief Disable backtrace logging.
     */
    void disableBacktrace();

    /**
     * @brief Dump all backtrace messages to the log.
     * Useful for debugging after a critical error occurs.
     */
    void dumpBacktrace();

    /**
     * @brief Get access to the underlying spdlog logger.
     * @return Shared pointer to the spdlog logger instance.
     * @note Use with caution - prefer using the wrapper methods.
     */
    std::shared_ptr<spdlog::logger> getLogger();

    /**
     * @brief Shutdown the logger, flushing all pending messages and closing files.
     * @note This is automatically called in the destructor, but can be called manually.
     */
    void shutdown();

   private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> m_logger;
    bool m_initialized = false;
};

// Convenience macros for cleaner logging syntax
#define LOG_TRACE(...) Logging::Logger::instance().trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logging::Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...) Logging::Logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...) Logging::Logger::instance().warn(__VA_ARGS__)
#define LOG_ERROR(...) Logging::Logger::instance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logging::Logger::instance().critical(__VA_ARGS__)

// Convenience macro for flushing the logger
#define LOG_FLUSH() Logging::Logger::instance().flush()

}  // namespace Logging
