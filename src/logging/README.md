# Logging Module with spdlog

This module uses [spdlog](https://github.com/gabime/spdlog) for fast, efficient logging throughout the CAN Bus Manager application.

## Features

- **Multiple log levels**: trace, debug, info, warn, error, critical
- **Console and file output**: Logs to both stdout (colored) and rotating log files
- **Thread-safe**: Safe for use in multi-threaded environments
- **Automatic log rotation**: Prevents log files from growing too large
- **High performance**: Minimal overhead with asynchronous logging capabilities
- **Backtrace support**: Store recent log messages for debugging critical errors
- **Customizable patterns**: Configure log message formatting
- **Proper shutdown**: Graceful cleanup with flush guarantees

## Usage

### Basic Logging

```cpp
#include "logging/logger.hpp"

// Using convenience macros (recommended)
LOG_INFO("Application started");
LOG_DEBUG("Processing message ID: 0x{:X}", messageId);
LOG_WARN("Buffer nearly full: {}%", percentage);
LOG_ERROR("Failed to open file: {}", filename);
LOG_CRITICAL("System crash imminent!");

// Using the Logger instance directly
Logging::Logger::instance().info("Starting operation");
Logging::Logger::instance().error("Operation failed with error: {}", error);
```

### Initialization

The logger is automatically initialized in the LoggingComponent constructor with default settings:

```cpp
Logger::instance().initialize("logs/logging_module.log");
```

You can customize initialization:

```cpp
// Custom log file path, max size (5MB), max files (3)
Logger::instance().initialize("logs/my_custom.log", 1024 * 1024 * 5, 3);
```

### Setting Log Level

```cpp
// Set globally for all loggers
Logger::instance().setLevel(spdlog::level::debug);

// Available levels:
// - spdlog::level::trace    (most verbose)
// - spdlog::level::debug
// - spdlog::level::info     (default)
// - spdlog::level::warn
// - spdlog::level::err
// - spdlog::level::critical
// - spdlog::level::off      (disable logging)

// Get current log level
auto currentLevel = Logger::instance().getLevel();
```

### Custom Log Pattern

```cpp
// Change the log message format pattern
Logger::instance().setPattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

// Common pattern placeholders:
// %Y-%m-%d %H:%M:%S.%e : date and time with milliseconds
// %l : log level (info, warn, error, etc.)
// %n : logger name
// %v : the actual text to log
// %s : source file name
// %# : line number
// %! : function name
// %t : thread id
// %^ and %$ : color range (for console)
```

### Backtrace for Debugging

```cpp
// Enable backtrace to store the last 32 messages in memory
Logger::instance().enableBacktrace(32);

// When a critical error occurs, dump the backtrace
LOG_ERROR("Critical error detected");
Logger::instance().dumpBacktrace();  // Outputs last 32 messages

// Disable backtrace when no longer needed
Logger::instance().disableBacktrace();
```

### Flushing and Shutdown

```cpp
// Flush all pending log messages immediately
Logger::instance().flush();
// Or use the convenience macro
LOG_FLUSH();

// Properly shutdown the logger (automatically called on destruction)
Logger::instance().shutdown();
```

### Checking Initialization Status

```cpp
if (Logger::instance().isInitialized()) {
    LOG_INFO("Logger is ready");
} else {
    // Logger is using default console-only configuration
}
```

### Formatting

spdlog uses the [fmt library](https://github.com/fmtlib/fmt) for formatting:

```cpp
// Numbers
LOG_INFO("Count: {}", count);
LOG_INFO("Hex: 0x{:X}", value);
LOG_INFO("Float: {:.2f}", pi);

// Strings
LOG_INFO("Name: {}, Age: {}", name, age);

// Padding and alignment
LOG_INFO("{:<10} | {:>10}", "Left", "Right");
```

## Log Files

Logs are written to:
- **Console**: Colored output with format `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message`
- **File**: `logs/logging_module.log` with format `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [logger_name] [thread ID] message`

Log rotation occurs automatically:
- `logging_module.log` (current log)
- `logging_module.1.log` (previous rotation)
- `logging_module.2.log` (older rotation)

## Examples from the Logging Module

### Component Lifecycle

```cpp
void LoggingComponent::onStart()
{
    LOG_INFO("Starting LoggingComponent");
    // ... setup code ...
}

void LoggingComponent::onStop()
{
    LOG_INFO("Stopping LoggingComponent");
    stopLogging();
    Logger::instance().flush();  // Ensure all logs are written
}
```

### Session Management

```cpp
void LoggingComponent::startLogging()
{
    LOG_INFO("Starting new logging session on device: {}", selectedDevice.toStdString());
    m_model->startNewSession(selectedDevice);
}

void LoggingModel::stopActiveSession()
{
    LOG_INFO("Session stopped - ID: {}, Duration: {}, Total messages: {}",
             sessionId, duration, entryCount);
}
```

### Error Handling

```cpp
bool LoggingComponent::writeToCsv(const QString& sessionId, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_ERROR("Failed to open file for writing: {}", filePath.toStdString());
        return false;
    }
    // ... write data ...
    LOG_INFO("Successfully exported session {} to {}", sessionId, filePath);
    return true;
}
```

### Trace-Level Debugging

```cpp
// Very detailed logging for debugging (only shown when log level is trace)
m_rawMsgConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
    [this](const Core::ReceivedCanRawEvent& event) {
        LOG_TRACE("Received raw CAN frame - ID: 0x{:X}", 
                  static_cast<uint8_t>(event.canMessage.messageId));
        emit receiveRawFrame(event.canMessage);
    });
```

### Backtrace for Post-Mortem Debugging

```cpp
void CriticalComponent::initialize() {
    // Enable backtrace at startup for critical components
    Logger::instance().enableBacktrace(50);
}

void CriticalComponent::onCriticalError(const std::exception& ex) {
    LOG_CRITICAL("Critical error occurred: {}", ex.what());
    
    // Dump the last 50 log messages to help diagnose the issue
    Logger::instance().dumpBacktrace();
    
    // Flush to ensure everything is written
    Logger::instance().flush();
}
```

## Best Practices

1. **Choose appropriate log levels**:
   - `TRACE`: Very detailed debugging info (e.g., every CAN frame)
   - `DEBUG`: Diagnostic information useful during development
   - `INFO`: General informational messages about program operation
   - `WARN`: Warning messages for potentially harmful situations
   - `ERROR`: Error events that might still allow the application to continue
   - `CRITICAL`: Very severe errors that will likely cause the application to abort

2. **Flush logs on shutdown**: Call `Logger::instance().flush()` before exit to ensure all logs are written

3. **Use structured logging**: Include context in your log messages
   ```cpp
   LOG_INFO("Session {} started on device {}", sessionId, deviceName);
   ```

4. **Avoid excessive logging in hot paths**: Use `TRACE` level for high-frequency events

5. **Convert Qt strings**: Use `.toStdString()` when logging QString values
   ```cpp
   LOG_INFO("Device: {}", selectedDevice.toStdString());
   ```

6. **Use backtrace for debugging**: Enable backtrace in critical components to capture recent log history
   ```cpp
   Logger::instance().enableBacktrace(50);  // Store last 50 messages
   ```

7. **Proper shutdown**: Ensure proper cleanup in destructors
   ```cpp
   MyComponent::~MyComponent() {
       LOG_INFO("Shutting down component");
       Logger::instance().flush();
   }
   ```

8. **Check initialization**: Verify logger is initialized before critical operations
   ```cpp
   if (!Logger::instance().isInitialized()) {
       Logger::instance().initialize("logs/myapp.log");
   }
   ```

## Advanced Features

### Custom Formatting Patterns

The logger uses spdlog's pattern formatting. Common placeholders:
- `%Y-%m-%d`: Date (YYYY-MM-DD)
- `%H:%M:%S.%e`: Time with milliseconds
- `%l`: Log level
- `%n`: Logger name
- `%v`: Message
- `%t`: Thread ID
- `%s`: Source file
- `%#`: Line number
- `%!`: Function name
- `%^` and `%$`: Color range (console only)

### Accessing the Underlying Logger

For advanced use cases, you can access the underlying spdlog logger:

```cpp
auto spdlogger = Logger::instance().getLogger();
// Use spdlog's API directly (not recommended for normal use)
```

## Performance Considerations

- spdlog is designed for minimal overhead
- Console output is more expensive than file output
- Consider using asynchronous logging for high-throughput scenarios
- Log files automatically rotate to prevent disk space issues

## Thread Safety

The logger is thread-safe and can be called from any thread without additional synchronization.
