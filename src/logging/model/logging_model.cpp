#include "logging_model.hpp"

#include "core/util/log_service.hpp"
#include "logging/logger.hpp"

namespace Logging {
// Initialize base class and members
// Constructs the logging data model
LoggingModel::LoggingModel(QObject* parent) : QAbstractTableModel(parent), m_activeSessionIndex(-1)
{
}

// Returns the number of logging sessions (rows)
int LoggingModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_sessions.size());
}

// Returns the number of columns in the table
int LoggingModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return Col_MAX;
}

// Returns data for a specific cell (timestamp, duration, etc.)
QVariant LoggingModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size()))
    {
        return QVariant();
    }
    const LogSession& session = m_sessions[index.row()];

    // Handle display and custom roles
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case Col_Timestamp:
                // Show timestamp as milliseconds since Unix epoch (system time)
                return static_cast<qulonglong>(session.startDateTime.toMSecsSinceEpoch());
            case Col_Duration:
                return session.duration;
            case Col_Signals:
                return QVariant();  // Signals will be painted by delegate
            case Col_Actions:
                return QVariant();  // Actions will be painted by delegate
            default:
                return QVariant();
        }
        // Custom roles
    } else if (role == SessionIdRole)
    {
        return session.id;
    } else if (role == IsActiveRole)
    {
        return session.isRecording;
    } else if (role == EntryCountRole)
    {
        return static_cast<qulonglong>(session.entryCount);
    } else if (role == SignalsListRole)
    {
        return session.messageSignals;
    }
    return QVariant();
}

// Returns column header labels
QVariant LoggingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    switch (section)
    {
        case Col_Timestamp:
            return QString("Timestamp");
        case Col_Duration:
            return QString("Duration");
        case Col_Signals:
            return QString("Message IDs");
        case Col_Actions:
            return QString("Actions");
        default:
            return QVariant();
    }
    return QVariant();
}

// Retrieves a session by its unique ID
const Logging::LogSession* LoggingModel::getSession(const QString& sessionId) const
{
    for (const auto& session : m_sessions)
    {
        if (session.id == sessionId)
        {
            return &session;
        }
    }
    return nullptr;
}

// Returns the session ID for a given table row index
QString LoggingModel::sessionIdAt(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size()))
    {
        return QString();
    }
    return m_sessions[index.row()].id;
}

// Returns true if a logging session is currently active
[[nodiscard]] bool LoggingModel::isRecording() const
{
    return m_activeSessionIndex != -1;
}

// Updates the stored DBC configuration reference
void LoggingModel::updateDbcConfig(const Core::DbcConfig& config)
{
    m_currentDbc = config;  // Store a copy in std::optional
}

// Returns the current DBC configuration if available
const std::optional<Core::DbcConfig>& LoggingModel::getCurrentDbcConfig() const
{
    return m_currentDbc;
}

// Creates and starts a new logging session with selected signals
void LoggingModel::startNewSession(const QString& deviceName,
                                   const std::map<uint32_t, QStringList>& selectedSignals)
{
    if (isRecording())
    {
        LOG_WARN("Stopping previous session before starting new one");
        stopActiveSession();
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    LogSession newSession;
    newSession.id = QString::number(QDateTime::currentMSecsSinceEpoch());
    newSession.startDateTime = QDateTime::currentDateTime();
    newSession.duration = "00:00:00";
    newSession.deviceName = deviceName;
    newSession.isRecording = true;
    newSession.entryCount = 0;
    newSession.selectedSignals = selectedSignals;

    // Build initial message signals list from selected signals
    for (const auto& [messageId, signalList] : selectedSignals)
    {
        if (!signalList.isEmpty())
        {
            QString messageIdStr = QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();
            newSession.messageSignals.append(messageIdStr);
        }
    }

    m_sessions.push_back(std::move(newSession));
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();

    // Create log file for this session
    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    QString logFileName = QString("logs/session_%1.csv").arg(activeSession.id);
    activeSession.logFile = std::make_unique<QFile>(logFileName);

    if (activeSession.logFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        activeSession.logStream = std::make_unique<QTextStream>(activeSession.logFile.get());

        // Write header based on whether DBC is loaded
        if (m_currentDbc.has_value())
        {
            *activeSession.logStream << "Timestamp,Message ID,Message Name,Signal,Value\n";
        } else
        {
            *activeSession.logStream << "Timestamp,CAN ID,DLC,Data\n";
        }
        activeSession.logStream->flush();
        LOG_INFO("Log file created: {}", logFileName.toStdString());
    } else
    {
        LOG_ERROR("Failed to create log file: {}", logFileName.toStdString());
    }

    LOG_INFO("New logging session started - ID: {}, Device: {}, Messages with signals: {}",
             newSession.id.toStdString(), deviceName.toStdString(),
             static_cast<int>(selectedSignals.size()));
}

// Stops the currently active logging session
void LoggingModel::stopActiveSession()
{
    if (!isRecording())
    {
        return;
    }

    const QString sessionId = m_sessions[m_activeSessionIndex].id;
    const uint64_t entryCount = m_sessions[m_activeSessionIndex].entryCount;
    const QString duration = m_sessions[m_activeSessionIndex].duration;

    // Close log file and clean up
    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    if (activeSession.logStream)
    {
        activeSession.logStream->flush();
        activeSession.logStream.reset();
    }
    if (activeSession.logFile)
    {
        activeSession.logFile->close();
        LOG_INFO("Log file closed for session: {}", sessionId.toStdString());
    }

    m_sessions[m_activeSessionIndex].isRecording = false;
    updateActiveDuration();

    m_activeSessionIndex = -1;

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

    LOG_INFO("Session stopped - ID: {}, Duration: {}, Total messages: {}", sessionId.toStdString(),
             duration.toStdString(), entryCount);
}

// Updates the elapsed duration of the active logging session
void LoggingModel::updateActiveDuration()
{
    if (!isRecording())
    {
        return;
    }
    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    QDateTime now = QDateTime::currentDateTime();
    QDateTime start = activeSession.startDateTime;
    qint64 seconds = start.secsTo(now);

    QTime durationTime(0, 0);
    durationTime = durationTime.addSecs(static_cast<int>(seconds));
    activeSession.duration = durationTime.toString("hh:mm:ss");

    // Notify views that the duration has changed (timestamp column doesn't change)
    QModelIndex durationIndex = index(m_activeSessionIndex, Col_Duration);
    emit dataChanged(durationIndex, durationIndex);
}

// slots
// Handles incoming raw CAN frames and updates entry count
void LoggingModel::onRawFrameReceived(const Core::RawCanMessage& msg)
{
    if (!isRecording() || m_currentDbc.has_value())
    {
        return;
    }

    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    activeSession.entryCount++;

    // Write to log file
    if (activeSession.logStream)
    {
        *activeSession.logStream << msg.receiveTime.count() << ","
                                 << QString("0x%1").arg(msg.messageId, 3, 16, QChar('0')).toUpper()
                                 << "," << static_cast<int>(msg.dlc) << ",\"";

        // Write hex data
        for (int i = 0; i < msg.dlc && i < 8; i++)
        {
            if (i > 0) *activeSession.logStream << " ";
            *activeSession.logStream
                << QString("%1")
                       .arg(static_cast<uint8_t>(msg.data[i]), 2, 16, QChar('0'))
                       .toUpper();
        }
        *activeSession.logStream << "\"\n";
        activeSession.logStream->flush();
    }

    // Add message ID to signals list if not already present
    QString messageId = QString("0x%1").arg(msg.messageId, 3, 16, QChar('0')).toUpper();
    if (!activeSession.messageSignals.contains(messageId))
    {
        activeSession.messageSignals.append(messageId);
        LOG_DEBUG("New raw message logged: {}", messageId.toStdString());
        emit dataChanged(index(m_activeSessionIndex, Col_Signals),
                         index(m_activeSessionIndex, Col_Signals));
    }
}

// Handles incoming DBC decoded messages with signal filtering
void LoggingModel::onDbcSignalsReceived(const Core::DbcCanMessage& msg)
{
    if (!isRecording() || !m_currentDbc.has_value())
    {
        return;
    }

    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    uint32_t messageId = static_cast<uint32_t>(msg.messageId);

    // Check if this message has selected signals
    auto it = activeSession.selectedSignals.find(messageId);
    if (it == activeSession.selectedSignals.end() || it->second.isEmpty())
    {
        // No signals selected for this message, skip logging
        return;
    }

    activeSession.entryCount++;

    // Write to log file
    if (activeSession.logStream)
    {
        // Find message name from DBC config
        QString messageName = QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();
        for (const auto& msgDef : m_currentDbc->messageDefinitions)
        {
            if (msgDef.messageId == messageId)
            {
                messageName = QString::fromStdString(msgDef.messageName);
                break;
            }
        }

        // Write each signal value
        for (const auto& signal : msg.signalValues)
        {
            *activeSession.logStream
                << msg.receiveTime.count() << ","
                << QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper() << "," << messageName
                << "," << QString::fromStdString(signal.name) << "," << signal.value << "\n";
        }
        activeSession.logStream->flush();
    }

    // Add message ID to signals list if not already present
    QString signalName = QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();

    if (!activeSession.messageSignals.contains(signalName))
    {
        activeSession.messageSignals.append(signalName);
        LOG_DEBUG("New DBC signal logged: {} | {}", signalName.toStdString(), msg.messageId);
        emit dataChanged(index(m_activeSessionIndex, Col_Signals),
                         index(m_activeSessionIndex, Col_Signals));
    } else
    {
        LOG_DEBUG("Known DBC signal logged: {} | {}", signalName.toStdString(), msg.messageId);
    }
}
}  // namespace Logging
