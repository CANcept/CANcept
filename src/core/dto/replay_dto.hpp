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

#include <QString>
#include <array>
#include <chrono>
#include <cstdint>

namespace Core {

// ============================================================================
// ENUMS
// ============================================================================

/**
 * @enum ReplaySessionType
 * @brief Type of log session (RAW or DBC_BASED)
 */
enum class ReplaySessionType { RAW, DBC_BASED };


// ============================================================================
// DATA TRANSFER OBJECTS
// ============================================================================

/**
 * @struct ReplaySessionInfo
 * @brief Metadata for a single replay session
 *
 * Used for Logging module to advertise available sessions to Sending module.
 */
struct ReplaySessionInfo {
    QString sessionId;  // Unique session identifier (typically timestamp-based)
    QString displayName;  // Human-readable name (e.g., formatted datetime)
    size_t frameCount = 0;  // Total number of frames in session
    std::chrono::milliseconds duration{0};  // Total session duration
    ReplaySessionType type = ReplaySessionType::RAW;  // RAW or DBC_BASED
};

/**
 * @struct ReplayFrame
 * @brief A single CAN message from a replay session
 *
 * Time is RELATIVE to session start (first frame ≈ 0ms).
 * Sending module converts this to absolute time when scheduling.
 */
struct ReplayFrame {
    std::chrono::milliseconds receiveTime{0};  // Relative time from session start
    uint16_t messageId = 0;  // CAN message ID (11-bit)
    uint8_t dlc = 8;  // Data Length Code (0-8 bytes)
    std::array<uint8_t, 8> data = {};  // Message payload
};

}  // namespace Core