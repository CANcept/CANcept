#pragma once

#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <map>
#include <optional>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"

namespace Logging {

enum LogSessionType { RAW, DBC_BASED };
/** * @struct LogSession
 * @brief Represents a complete recording period with metadata.
 *
 * Note: Entry counts and message lists are NOT tracked in memory.
 * They are read from the log file (logs/session_{id}_CanLogging.log) when needed.
 */
struct LogSession {
    QString id;                // Session ID (timestamp) - used to locate log file
    QDateTime startDateTime;   // When session started
    QString duration;          // Session duration (HH:MM:SS)
    bool isRecording = false;  // Whether session is currently active
    LogSessionType type;       // Type of the Session
    std::map<uint32_t, QStringList> selectedSignals;  // Map of message ID to selected signal names
                                                      // (for filtering during logging)
    std::map<uint16_t, std::pair<int, int>> signalsBeforeAfterMessage;
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
     * @brief Creates a new session and sets it as the active target for data.
     * @param selectedSignals Map of message IDs to selected signal names for logging.
     * @param signalsBeforeAfterMessage
     */
    void startNewDbcLogSession(
        const std::map<uint32_t, QStringList>& selectedSignals = {},
        const std::map<uint16_t, std::pair<int, int>>& signalsBeforeAfterMessage = {});

    void startNewRawLogsSession();
    /**
     * @brief Finalizes the active session, locking it for export.
     */
    void stopActiveSession();

    /** @brief Updates the duration string of the active session based on current time. */
    void updateActiveDuration();

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

   private:
    std::optional<Core::DbcConfig> m_currentDbc;

    std::vector<LogSession> m_sessions;
    int m_activeSessionIndex = -1;
    std::mutex m_messageReceiveMutex;
};

}  // namespace Logging