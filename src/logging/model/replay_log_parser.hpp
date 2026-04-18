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

#include <cstdint>
#include <map>

#include <QString>
#include <QStringList>
#include <chrono>

#include "core/dto/can_dto.hpp"
#include "core/dto/replay_dto.hpp"

namespace Logging {

/**
 * @brief Stateless helper for replay metadata and RAW log frame parsing.
 */
class ReplayLogParser final
{
   public:
    static auto parseDurationToMs(const QString& duration) -> std::chrono::milliseconds;

    static auto countLogEntries(const QString& logFilePath) -> size_t;

    static auto parseRawReplayFrames(const QString& logFilePath, QList<Core::ReplayFrame>& frames,
                                     size_t& skippedFrameCount, QString& errorMessage) -> bool;

    static auto parseDbcReplayMessages(
        const QString& logFilePath,
        const std::map<uint32_t, QStringList>& selectedSignalsPerMessage,
        const std::map<uint16_t, std::pair<int, int>>& signalsBeforeAfterMessage,
        QList<Core::DbcCanMessage>& messages,
        size_t& skippedFrameCount,
        QString& errorMessage) -> bool;
};

}  // namespace Logging

