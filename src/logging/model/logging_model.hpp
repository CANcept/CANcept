#pragma once

#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <map>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/dto/dbc_dto.hpp"

namespace Logging {

/** * @struct LogSession
 * @brief Represents a complete recording period with metadata and captured data.
 */
struct LogSession {
    QString id;
    QDateTime startDateTime;
    QString duration;
    bool isRecording = false;
    QString deviceName;
    uint64_t entryCount = 0;
};

/**
 * @class LoggingModel
 * @brief The central data authority for the Logging module.
 * * @details
 * This model manages the lifecycle of logging sessions. It acts as a, list and data provider.
 */
class LoggingModel final : public QAbstractTableModel
{
    Q_OBJECT

   public:
    /**
     * @enum Roles
     * @brief Custom roles for accessing session-specific data.
     */
    enum Roles { SessionIdRole = Qt::UserRole + 1, SessionDataRole, IsActiveRole, EntryCountRole };

    /** @brief Column definitions for the History Table. */
    enum Columns {
        Col_ID = 0,
        Col_StartTime,
        Col_Duration,
        Col_Count,
        Col_Actions,  // New column for Delegate-painted buttons
        Col_MAX
    };

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

    void updateDbcConfig(const Core::DbcConfig& config);

   public slots:
    /** @brief Triggered by Component's bridge signal */
    void onRawFrameReceived(const Core::RawCanMessage& msg);

    /** @brief Triggered by Component's bridge signal */
    void onDbcSignalsReceived(const Core::DbcCanMessage& msg);

    /**
     * @brief Creates a new session and sets it as the active target for data.
     * @param deviceName The hardware interface used for this session.
     */
    void startNewSession(const QString& deviceName);

    /**
     * @brief Finalizes the active session, locking it for export.
     */
    void stopActiveSession();

   private:
    /** @brief Updates the duration string of the active session based on current time. */
    void updateActiveDuration();

    Core::DbcConfig* m_currentDbc;

    std::vector<LogSession> m_sessions;
    int m_activeSessionIndex = -1;
};

}  // namespace Logging