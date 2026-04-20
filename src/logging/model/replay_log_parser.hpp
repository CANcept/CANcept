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
#include <QStringList>
#include <chrono>
#include <cstdint>
#include <map>

#include "core/dto/can_dto.hpp"
#include "core/dto/replay_dto.hpp"

namespace Logging {

/**
 * @class ReplayLogParser
 * @brief Stateless helper for replay metadata parsing and log-to-replay transformation.
 *
 * This utility class converts persisted logging artifacts into replay-ready DTOs.
 * It supports both RAW log files (direct frame parsing) and DBC-based log files
 * (signal-row reconstruction into decoded DBC messages).
 */
class ReplayLogParser final
{
   public:
    /**
     * @brief Parses a duration string in format "hh:mm:ss" into milliseconds.
     * @param duration Duration string as stored by the logging model.
     * @return Parsed duration in milliseconds, or 0 ms if the input is invalid.
     */
    static auto parseDurationToMs(const QString& duration) -> std::chrono::milliseconds;

    /**
     * @brief Counts non-empty data rows in a log file (header excluded).
     * @param logFilePath Path to the persisted log file.
     * @return Number of replayable data rows (best-effort count).
     */
    static auto countLogEntries(const QString& logFilePath) -> size_t;

    /**
     * @brief Parses a RAW log file into replay frames.
     * @param logFilePath Path to the RAW log file.
     * @param frames Output list receiving successfully parsed replay frames.
     * @param skippedFrameCount Output counter for invalid lines skipped during parsing.
     * @param errorMessage Output error text for hard failures (e.g. file open failure).
     * @return True if parsing completed, false on hard failure.
     */
    static auto parseRawReplayFrames(const QString& logFilePath, QList<Core::ReplayFrame>& frames,
                                     size_t& skippedFrameCount, QString& errorMessage) -> bool;

    /**
     * @brief Parses a DBC-based log file into decoded DBC messages.
     * @param logFilePath Path to the DBC log file.
     * @param selectedSignalsPerMessage Map of message IDs to selected signal names.
     * @param signalsBeforeAfterMessage Column offset metadata used to locate per-message signal
     * blocks in each CSV row.
     * @param messages Output list receiving successfully reconstructed DBC messages.
     * @param skippedFrameCount Output counter for invalid or ambiguous rows.
     * @param errorMessage Output error text for hard failures (e.g. file open failure).
     * @return True if parsing completed, false on hard failure.
     */
    static auto parseDbcReplayMessages(
        const QString& logFilePath,
        const std::map<uint32_t, QStringList>& selectedSignalsPerMessage,
        const std::map<uint16_t, std::pair<int, int>>& signalsBeforeAfterMessage,
        QList<Core::DbcCanMessage>& messages, size_t& skippedFrameCount,
        QString& errorMessage) -> bool;
};

}  // namespace Logging
