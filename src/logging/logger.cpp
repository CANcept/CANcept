#include "logger.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

namespace Logging {

// Returns the singleton logger instance
Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

// Constructs logger with default console sink
Logger::Logger()
{
    // Create a default console-only logger until initialize() is called
    m_logger = spdlog::stdout_color_mt("logging_module");
    m_logger->set_level(spdlog::level::info);
    m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
}

// Destroys logger and performs cleanup
Logger::~Logger()
{
    shutdown();
}

// Initializes logger with file and console sinks
void Logger::initialize(const std::string& logFilePath, size_t maxFileSize, size_t maxFiles)
{
    if (m_initialized)
    {
        m_logger->warn("Logger already initialized. Skipping re-initialization.");
        return;
    }

    try
    {
        // Ensure the log directory exists
        std::filesystem::path logPath(logFilePath);
        std::filesystem::create_directories(logPath.parent_path());

        // Create sinks: console (stdout) + rotating file
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath, maxFileSize, maxFiles);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [thread %t] %v");

        // Combine sinks into a multi-sink logger
        spdlog::sinks_init_list sink_list = {console_sink, file_sink};

        // Drop the old logger and create a new one with both sinks
        spdlog::drop("logging_module");
        m_logger =
            std::make_shared<spdlog::logger>("logging_module", sink_list.begin(), sink_list.end());
        m_logger->set_level(spdlog::level::trace);
        m_logger->flush_on(spdlog::level::warn);  // Auto-flush on warnings and above

        // Register the logger with spdlog's registry
        spdlog::register_logger(m_logger);

        m_initialized = true;
        m_logger->info("Logger initialized successfully. Log file: {}", logFilePath);
    } catch (const spdlog::spdlog_ex& ex)
    {
        // Fallback to console-only logging
        try
        {
            spdlog::drop("logging_module_fallback");
            m_logger = spdlog::stdout_color_mt("logging_module_fallback");
            m_logger->set_level(spdlog::level::info);
            m_logger->error("Logger initialization failed: {}", ex.what());
        } catch (const std::exception& fallbackEx)
        {
            // If even fallback fails, use default logger
            m_logger = spdlog::default_logger();
            m_logger->error("Logger fallback initialization failed: {}", fallbackEx.what());
        }
        m_initialized = false;
    } catch (const std::exception& ex)
    {
        // Catch any other exceptions
        m_logger = spdlog::default_logger();
        m_logger->error("Logger initialization failed with unexpected error: {}", ex.what());
        m_initialized = false;
    }
}

// Sets the minimum log level for filtering messages
void Logger::setLevel(spdlog::level::level_enum level)
{
    if (m_logger)
    {
        m_logger->set_level(level);
        m_logger->info("Log level changed to: {}", spdlog::level::to_string_view(level));
    }
}

// Returns the current log level
spdlog::level::level_enum Logger::getLevel() const
{
    if (m_logger)
    {
        return m_logger->level();
    }
    return spdlog::level::info;  // Default level
}

// Sets the log message formatting pattern
void Logger::setPattern(const std::string& pattern)
{
    if (m_logger)
    {
        m_logger->set_pattern(pattern);
        m_logger->debug("Log pattern changed");
    }
}

// Returns true if logger has been initialized with file sink
bool Logger::isInitialized() const
{
    return m_initialized;
}

// Flushes all buffered log messages to file
void Logger::flush()
{
    if (m_logger)
    {
        m_logger->flush();
    }
}

// Enables backtrace logging for debugging (stores recent messages)
void Logger::enableBacktrace(size_t numMessages)
{
    if (m_logger)
    {
        m_logger->enable_backtrace(numMessages);
        m_logger->debug("Backtrace enabled with {} message buffer", numMessages);
    }
}

// Disables backtrace logging
void Logger::disableBacktrace()
{
    if (m_logger)
    {
        m_logger->disable_backtrace();
        m_logger->debug("Backtrace disabled");
    }
}

// Dumps stored backtrace messages to log output
void Logger::dumpBacktrace()
{
    if (m_logger)
    {
        m_logger->dump_backtrace();
    }
}

// Returns the underlying spdlog logger instance
std::shared_ptr<spdlog::logger> Logger::getLogger()
{
    return m_logger;
}

// Shuts down and releases all logger resources
void Logger::shutdown()
{
    if (m_logger)
    {
        m_logger->flush();
        m_logger->info("Logger shutting down");
    }

    // Drop all registered loggers
    spdlog::drop("logging_module");
    spdlog::drop("logging_module_fallback");

    // Shutdown spdlog completely
    spdlog::shutdown();
}

}  // namespace Logging
