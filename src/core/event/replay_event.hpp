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
#include <QList>

#include "core/dto/replay_dto.hpp"
#include "event.hpp"

namespace Core {

// ============================================================================
// REQUEST EVENTS (Sending -> Logging)
// ============================================================================

/**
 * @brief Request: Get all frames from a specific log session.
 *
 * Sending publishes this when user selects and loads a session.
 * Logging responds with SendLogSessions.
 */
struct LogSessionFramesRequest final : public Event {
    QString sessionId;

    explicit LogSessionFramesRequest(const QString& id) : sessionId(id) {}
};

// ============================================================================
// UPDATE/RESPONSE EVENTS (Logging -> Sending)
// ============================================================================

/**
 * @brief Push update: Current list of replayable (finished) sessions.
 *
 * Logging publishes this automatically when replay session availability changes
 * (e.g. after stopping a recording) and once on startup.
 */
struct SendLogSessions final : public Event {
    QList<ReplaySessionInfo> sessions;
    QString errorMessage;  // Empty if success, text if failed

    explicit SendLogSessions(
        const QList<ReplaySessionInfo>& s = QList<ReplaySessionInfo>())
        : sessions(s)
    {
    }
};

/**
 * @brief Response: Frames from a replay session.
 *
 * Logging publishes this in response to LogSessionFramesRequest.
 */
struct SendLogSessionFrames final : public Event {
    QString sessionId;
    QList<ReplayFrame> frames;
    QString errorMessage;  // Empty if success, text if failed
    size_t skippedFrameCount = 0;  // Number of invalid frames skipped during parsing

    explicit SendLogSessionFrames(const QString& id = QString())
        : sessionId(id)
    {
    }
};

}  // namespace Core
