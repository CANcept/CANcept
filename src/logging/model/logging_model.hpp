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

#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <atomic>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"
#include "logging/worker/logging_worker.hpp"

namespace Logging {

enum LogSessionType { RAW, DBC_BASED };
/** * @struct LogSession
 * @brief Represents a complete recording period with metadata.
 *
 * Note: Entry counts and message lists are NOT tracked in memory.
 * They are read from the log file (logs/session_{id}_CanLogging.log) when needed.
 */
struct LogSession {
    QString id;
    QDateTime startDateTime;
    QString duration;
    bool isRecording = false;
    LogSessionType type;
    std::map<uint32_t, QStringList> selectedSignals;

    std::unique_ptr<LoggingWorker> worker;
};

/**
 * @class LoggingModel
 * @brief The central data authority for the Logging module.
 * * @details
 * This model manages the lifecycle of logging sessions. It acts as a list and data provider.
 */
class LoggingModel final : public QAbstractTableModel
{
    Q_OBJECT

   public:
    /**
     * @enum Roles
     * @brief Custom roles for accessing session-specific data.
     */
    enum Roles {
        SessionIdRole = Qt::UserRole + 1,
        SessionDataRole,
        IsActiveRole,
        EntryCountRole,
        SignalsListRole
    };

    /** @brief Column definitions for the History Table. */
    enum Columns { Col_Timestamp = 0, Col_Duration, Col_Signals, Col_Actions, Col_MAX };

    explicit LoggingModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /**
     * @brief Fetches a session by its unique ID.
     */
    const LogSession* getSession(const QString& sessionId) const;

    /**
     * @brief Helper to get the session ID from a View index.
     */
    QString sessionIdAt(const QModelIndex& index) const;

    /**
     * @brief Checks if a recording is currently in progress.
     */
    [[nodiscard]] bool isRecording() const;

    /**
     * @brief Returns the session ID of the currently active session.
     * @return Session ID or empty string if no active session.
     */
    QString getCurrentSessionId() const;

    /**
     * @brief Looks up message name from DBC config.
     * @param messageId CAN message ID.
     * @return Message name or "UNKNOWN" if not found.
     */
    QString getMessageName(uint16_t messageId) const;

    /**
     * @brief Looks up signal unit from DBC config.
     * @param messageId CAN message ID.
     * @param signalName Signal name.
     * @return Signal unit or empty string if not found.
     */
    QString getSignalUnit(uint16_t messageId, const QString& signalName) const;

    /**
     * @brief Gets the list of selected signals for a specific message.
     * @param messageId CAN message ID.
     * @return List of selected signal names (empty if no selection).
     */
    QStringList getSelectedSignalsForMessage(uint16_t messageId) const;

    void updateDbcConfig(const Core::DbcConfig& config);

    /**
     * @brief Creates a new DBC session and opens the MDF4 writer.
     * @param selectedSignals Map of message IDs to selected signal names for logging.
     */
    void startNewDbcLogSession(const std::map<uint32_t, QStringList>& selectedSignals = {});

    void startNewRawLogsSession();
    /**
     * @brief Finalizes the active session, locking it for export.
     */
    void stopActiveSession();

    /** @brief Updates the duration string of the active session based on current time. */
    void updateActiveDuration();

    /** @brief Returns the .mf4 file path for a given session ID. */
    static QString sessionFilePath(const QString& sessionId);

    /**
     * @brief Handler for incoming DBC-decoded CAN messages. Adds the message to the active session
     * if it's a DBC-based session.
     * @param message The received DBC-decoded CAN message to be logged.
     */
    void onDbcMessageReceived(const Core::DbcCanMessage& message);

    /**
     * @brief Handler for incoming raw CAN messages. Adds the message to the active session if it's
     * a RAW session.
     * @param message The received raw CAN message to be logged.
     */
    void onRawMessageReceived(const Core::RawCanMessage& message);

   public slots:
    void onExportRequested(const QModelIndex& index) const;

   private:
    std::optional<Core::DbcConfig> m_currentDbc;

    std::vector<LogSession> m_sessions;
    int m_activeSessionIndex = -1;

    // Hot-path worker pointers — loaded on every incoming message without a lock.
    // Written only on start/stop (UI thread), read from the event broker thread.
    std::atomic<LoggingWorker*> m_rawWorker{nullptr};
    std::atomic<LoggingWorker*> m_dbcWorker{nullptr};
};

}  // namespace Logging