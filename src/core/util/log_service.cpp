#include "log_service.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

#include "spdlog/sinks/basic_file_sink.h"

namespace Core {

// Private constructor - initializes spdlog async thread pool
LogService::LogService()
{
    spdlog::init_thread_pool(65536, 2);
    m_canThreadPool = std::make_shared<spdlog::details::thread_pool>(262144, 5);
}

// Private destructor - ensures clean shutdown
LogService::~LogService()
{
    flushAll();
    spdlog::shutdown();
}

// Retrieves or creates a logger for a specific context and session
std::shared_ptr<spdlog::logger> LogService::getLogger(const LogContext context,
                                                      const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_registryMutex);

    std::string key = createRegistryKey(context, sessionId);

    // Return existing logger if already registered
    if (const auto it = m_loggers.find(key); it != m_loggers.end())
    {
        return it->second;
    }

    // Create new logger based on context
    try
    {
        std::shared_ptr<spdlog::logger> logger;
        if (context == LogContext::CanLogging)
        {
            // Session-specific CAN data logger (file only)
            std::string logFilePath = getLogFilePath(context, sessionId);

            // Ensure logs directory exists
            std::filesystem::create_directories("logs");

            // Create rotating file sink (50MB max, 5 files for stress testing)
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath);

            file_sink->set_pattern("%v");

            // Create async logger with overrun policy (drops messages if queue full)
            logger = std::make_shared<spdlog::async_logger>(
                key, file_sink, m_canThreadPool, spdlog::async_overflow_policy::overrun_oldest);

            logger->set_level(spdlog::level::info);  // Info and above (skip trace/debug)
            logger->flush_on(spdlog::level::err);    // Only flush on errors (performance)
        } else                                       // LogContext::Debug
        {
            // System-wide debug logger (console + file)
            std::string logFilePath = getLogFilePath(context, sessionId);

            // Ensure logs directory exists
            std::filesystem::create_directories("logs");

            // Console sink with colors (restored original format)
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::debug);

            // Set color codes to match original console_logging.hpp style
            console_sink->set_color(spdlog::level::info, console_sink->green);
            console_sink->set_color(spdlog::level::warn, console_sink->yellow_bold);
            console_sink->set_color(spdlog::level::err, console_sink->red_bold);

            // Format: timestamp (dim) + level (colored) + context (cyan) + message
            console_sink->set_pattern(
                "\x1b[2m%H:%M:%S\x1b[0m %^[%l]\x1b[0m \x1b[1;36m[%n]\x1b[0m %v");

            // File sink
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFilePath, 1024 * 1024 * 5, 3);
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

            // Create SYNCHRONOUS logger for real-time debug output (no buffering delay)
            spdlog::sinks_init_list sinks = {console_sink, file_sink};
            logger = std::make_shared<spdlog::logger>(key, sinks.begin(), sinks.end());

            logger->set_level(spdlog::level::trace);
            // Synchronous logger flushes immediately, no need for flush_on
        }

        // Register logger
        spdlog::register_logger(logger);
        m_loggers[key] = logger;

        return logger;
    } catch (const spdlog::spdlog_ex& ex)
    {
        // Fallback to console-only logger on error
        auto fallback = spdlog::stdout_color_mt(key + "_fallback");
        fallback->error("Failed to create logger '{}': {}", key, ex.what());
        m_loggers[key] = fallback;
        return fallback;
    }
}

// Flushes all active loggers to disk
void LogService::flushAll()
{
    std::lock_guard<std::mutex> lock(m_registryMutex);

    for (auto& [key, logger] : m_loggers)
    {
        if (logger)
        {
            logger->flush();
        }
    }
}

// Closes a specific session logger and removes it from registry
void LogService::closeLogger(LogContext context, const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_registryMutex);

    const std::string key = createRegistryKey(context, sessionId);

    if (auto it = m_loggers.find(key); it != m_loggers.end())
    {
        // Flush before closing
        it->second->flush();

        // Drop from spdlog registry
        spdlog::drop(key);

        // Remove from our map
        m_loggers.erase(it);
    }
}

// Helper to generate unique registry key
auto LogService::createRegistryKey(LogContext context, const std::string& sessionId) -> std::string
{
    std::string contextStr = (context == LogContext::CanLogging) ? "CanLogging" : "Debug";
    return contextStr + "_" + sessionId;
}

auto LogService::getLogFilePath(const LogContext context,
                                const std::string& sessionId) -> std::string
{
    switch (context)
    {
        case LogContext::Debug:
            return "logs/system_debug.log";
        case LogContext::CanLogging:
            return "logs/session_" + sessionId + "_CanLogging.log";
        default:
            return "";
    }
}

}  // namespace Core
