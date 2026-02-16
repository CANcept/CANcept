#include "logging_model.hpp"

#include "core/util/log_service.hpp"

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
    switch (role)
    {
        case Qt::DisplayRole: {
            switch (index.column())
            {
                case Col_Timestamp:
                    return session.startDateTime.toString(
                        "HH:mm:ss");  // Show start time in 24h format (fixed)
                case Col_Duration:
                    return session.duration;
                case Col_Signals: {
                    switch (session.type)
                    {
                        case DBC_BASED: {
                            QStringList signalsList;
                            for (auto it = session.selectedSignals.begin();
                                 it != session.selectedSignals.end(); ++it)
                            {
                                signalsList.push_back(QString("0x") +
                                                      QString::number(it->first, 16));
                            }
                            return signalsList;
                        }
                        case RAW: {
                            return QStringList{QString("Raw")};
                        }
                        default:
                            return QVariant();
                    }
                }
                case Col_Actions:
                    return QVariant();  // Actions will be painted by delegate
                default:
                    return QVariant();
            }
        }
        case SessionIdRole: {
            return session.id;
        }
        case IsActiveRole: {
            return session.isRecording;
        }
        case EntryCountRole: {
            return static_cast<qulonglong>(0);
        }
        case SignalsListRole: {
            switch (session.type)
            {
                case DBC_BASED: {
                    QStringList signalsList;
                    for (auto it = session.selectedSignals.begin();
                         it != session.selectedSignals.end(); ++it)
                    {
                        signalsList.push_back(QString("0x") + QString::number(it->first, 16));
                    }
                    return signalsList;
                }
                case RAW: {
                    return QStringList{QString("Raw")};
                }
                default:
                    return QVariant();
            }
        }
        default: {
            return QVariant();
        }
    }
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
            return QString("Message-Id Signal");
        case Col_Actions:
            return QString("Actions");
        default:
            return QVariant();
    }
    return QVariant();
}

// Retrieves a session by its unique ID
const LogSession* LoggingModel::getSession(const QString& sessionId) const
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

// Returns the session ID of the currently active session
QString LoggingModel::getCurrentSessionId() const
{
    if (m_activeSessionIndex >= 0 && m_activeSessionIndex < static_cast<int>(m_sessions.size()))
    {
        return m_sessions[m_activeSessionIndex].id;
    }
    return QString();
}

// Looks up message name from DBC config
QString LoggingModel::getMessageName(uint16_t messageId) const
{
    if (!m_currentDbc.has_value())
    {
        return QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();
    }

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
    {
        if (msgDef.messageId == messageId)
        {
            return QString::fromStdString(msgDef.messageName);
        }
    }

    return QString("UNKNOWN_0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();
}

// Looks up signal unit from DBC config
QString LoggingModel::getSignalUnit(uint16_t messageId, const QString& signalName) const
{
    if (!m_currentDbc.has_value())
    {
        return QString();
    }

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
    {
        if (msgDef.messageId == messageId)
        {
            for (const auto& sigDef : msgDef.signalDescriptions)
            {
                if (QString::fromStdString(sigDef.signalName) == signalName)
                {
                    return QString::fromStdString(sigDef.unit);
                }
            }
        }
    }

    return QString();
}

// Gets the list of selected signals for a specific message
QStringList LoggingModel::getSelectedSignalsForMessage(uint16_t messageId) const
{
    if (m_activeSessionIndex < 0)
    {
        return QStringList();
    }

    const auto& activeSession = m_sessions[m_activeSessionIndex];
    auto it = activeSession.selectedSignals.find(static_cast<uint32_t>(messageId));

    if (it != activeSession.selectedSignals.end())
    {
        return it->second;
    }

    return QStringList();
}

// Updates the stored DBC configuration reference
void LoggingModel::updateDbcConfig(const Core::DbcConfig& config)
{
    m_currentDbc = config;  // Store a copy in std::optional
}

// Creates and starts a new logging session with selected signals
void LoggingModel::startNewDbcLogSession(
    const std::map<uint32_t, QStringList>& selectedSignals,
    const std::map<uint16_t, std::pair<int, int>>& signalsBeforeAfterMessage)
{
    if (isRecording())
    {
        stopActiveSession();
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    LogSession newSession;
    newSession.id = QString::number(QDateTime::currentMSecsSinceEpoch());
    newSession.startDateTime = QDateTime::currentDateTime();
    newSession.duration = "00:00:00";
    newSession.isRecording = true;
    newSession.selectedSignals = selectedSignals;
    newSession.signalsBeforeAfterMessage = signalsBeforeAfterMessage;
    newSession.type = DBC_BASED;

    m_sessions.push_back(newSession);
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();
}

void LoggingModel::startNewRawLogsSession()
{
    if (isRecording())
    {
        stopActiveSession();
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    LogSession newSession;
    newSession.id = QString::number(QDateTime::currentMSecsSinceEpoch());
    newSession.startDateTime = QDateTime::currentDateTime();
    newSession.duration = "00:00:00";
    newSession.isRecording = true;
    newSession.type = RAW;

    m_sessions.push_back(newSession);
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();
}

// Stops the currently active logging session
void LoggingModel::stopActiveSession()
{
    if (!isRecording())
    {
        return;
    }

    m_sessions[m_activeSessionIndex].isRecording = false;
    updateActiveDuration();

    m_activeSessionIndex = -1;

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
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

}  // namespace Logging
