#include "logging_model.hpp"

#include "core/util/log_service.hpp"

namespace Logging {
//Initialize base class and members    
LoggingModel::LoggingModel(QObject* parent)
    : QAbstractTableModel(parent), m_currentDbc(nullptr), m_activeSessionIndex(-1) {}


int LoggingModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_sessions.size());
}

int LoggingModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return Col_MAX;
}

QVariant LoggingModel::data(const QModelIndex& index, int role) const {
    
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size())) {
        return QVariant();
    }   
    const LogSession& session = m_sessions[index.row()];

    // Handle display and custom roles
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Col_ID:
                return session.id;
            case Col_StartTime:
                return session.startDateTime.toString(Qt::ISODate);
            case Col_Duration:
                return session.duration;
            case Col_Count:
                return static_cast<qulonglong>(session.entryCount);
            case Col_Actions:
                return QString("Actions");  // Placeholder for delegate-painted buttons
            default:
                return QVariant();
        }
        // Custom roles
        } else if (role == SessionIdRole) {
        return session.id;
        } else if (role == IsActiveRole) {
        return session.isRecording;
    } else if (role == EntryCountRole) {
        return static_cast<qulonglong>(session.entryCount);
    }
    return QVariant();
} 

QVariant LoggingModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {   
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    switch (section) {
            case Col_ID:
                return QString("Session ID");
            case Col_StartTime:
                return QString("Start Time");
            case Col_Duration:
                return QString("Duration");
            case Col_Count:
                return QString("Entry Count");
            case Col_Actions:
                return QString("Actions");
            default:
                return QVariant();
        }
    return QVariant();
}


const Logging::LogSession* LoggingModel::getSession(const QString& sessionId) const {
    for (const auto& session : m_sessions) {
        if (session.id == sessionId) {
            return &session;
        }
    }
    return nullptr;
}

QString LoggingModel::sessionIdAt(const QModelIndex& index) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size())) {
        return QString();
    }
    return m_sessions[index.row()].id;
}

[[nodiscard]] bool LoggingModel::isRecording() const {
    return m_activeSessionIndex != -1;
}

void LoggingModel::updateDbcConfig(const Core::DbcConfig& config) {
    m_currentDbc = const_cast<Core::DbcConfig*>(&config);
}

void LoggingModel::startNewSession(const QString& deviceName) {
    if (isRecording()) {
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

    m_sessions.push_back(newSession);
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();
}

void LoggingModel::stopActiveSession() {
    if (!isRecording()) {
        return;
    }
    m_sessions[m_activeSessionIndex].isRecording = false;
    updateActiveDuration();

    m_activeSessionIndex = -1;

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void LoggingModel::updateActiveDuration() {
    if (!isRecording()) {
        return;
    }
    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    QDateTime now = QDateTime::currentDateTime();
    QDateTime start = activeSession.startDateTime;
    qint64 seconds = start.secsTo(now);

    QTime durationTime(0, 0);
    durationTime = durationTime.addSecs(static_cast<int>(seconds));
    activeSession.duration = durationTime.toString("hh:mm:ss");

    // Notify views that the duration has changed
    QModelIndex durationIndex = index(m_activeSessionIndex, Col_Duration);
    emit dataChanged(durationIndex, durationIndex);
}

//slots
void LoggingModel::onRawFrameReceived(const Core::RawCanMessage& msg) {

    if (!isRecording()) {
        return;
    }
    m_sessions[m_activeSessionIndex].entryCount++;
    emit dataChanged(index(m_activeSessionIndex, Col_Count),
                     index(m_activeSessionIndex, Col_Count));
}

void LoggingModel::onDbcSignalsReceived(const Core::DbcCanMessage& msg) {
    if (!isRecording() || m_currentDbc == nullptr) {
        return;
    }
    m_sessions[m_activeSessionIndex].entryCount++;
    emit dataChanged(index(m_activeSessionIndex, Col_Count),
                     index(m_activeSessionIndex, Col_Count));
} 
} // namespace Logging
