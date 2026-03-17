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

#pragma once

#include <format>
#include <string>

#include "core/util/log_service.hpp"

/**
 * @file console_logging.hpp
 * @brief Centralized logging macros using Core::LogService.
 *
 * LOGGING_LEVEL values:
 * 0: Totally Silent (Production)
 * 1: Errors only
 * 2: Full Debug (Info, Warning, and Error)
 *
 * Logs to both console (colored) and file (logs/system_debug.log)
 */

#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL 0
#endif

/**
 * @brief Internal Helper Macro
 * Uses Core::LogService with Debug context for system-wide logging.
 * Maintains backwards compatibility with existing call sites.
 */
#define PRINT_LOG(level, cls, fmt_str, ...)                                               \
    {                                                                                     \
        std::string formatted_msg = std::format(fmt_str, ##__VA_ARGS__);                  \
        auto logger =                                                                     \
            Core::LogService::getInstance().getLogger(Core::LogContext::Debug, "system"); \
        std::string msg_with_context = std::format("[{}] {}", cls, formatted_msg);        \
        logger->log(level, msg_with_context);                                             \
    }

// --- Logic for Levels ---

// Level 1 and 2: Show Errors
#if LOGGING_LEVEL >= 1
#define LOG_ERR(cls, fmt_str, ...) PRINT_LOG(spdlog::level::err, cls, fmt_str, ##__VA_ARGS__)
#else
#define LOG_ERR(cls, fmt_str, ...)
#endif

// Level 2 only: Show Info and Warnings
#if LOGGING_LEVEL >= 2
#define LOG_INF(cls, fmt_str, ...) PRINT_LOG(spdlog::level::info, cls, fmt_str, ##__VA_ARGS__)
#define LOG_WRN(cls, fmt_str, ...) PRINT_LOG(spdlog::level::warn, cls, fmt_str, ##__VA_ARGS__)
#else
#define LOG_INF(cls, fmt_str, ...)
#define LOG_WRN(cls, fmt_str, ...)
#endif