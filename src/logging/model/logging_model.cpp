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

#include "logging_model.hpp"

#include <QFileDialog>
#include <filesystem>

#include "can_stream/can_converter.hpp"
#include "can_stream/writer/mdf4_writer.hpp"

namespace Logging {

LoggingModel::LoggingModel(QObject* parent) : QAbstractTableModel(parent), m_activeSessionIndex(-1)
{
}

int LoggingModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_sessions.size());
}

int LoggingModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return Col_MAX;
}

QVariant LoggingModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size())) return QVariant();

    const LogSession& session = m_sessions[index.row()];

    switch (role)
    {
        case Qt::DisplayRole: {
            switch (index.column())
            {
                case Col_Timestamp:
                    return session.startDateTime.toString("HH:mm:ss");
                case Col_Duration:
                    return session.duration;
                case Col_Signals: {
                    switch (session.type)
                    {
                        case DBC_BASED: {
                            QStringList list;
                            for (const auto& [id, _] : session.selectedSignals)
                                list.push_back(QString("0x") + QString::number(id, 16));
                            return list;
                        }
                        case RAW:
                            return QStringList{QString("Raw")};
                        default:
                            return QVariant();
                    }
                }
                case Col_Actions:
                    return QVariant();
                default:
                    return QVariant();
            }
        }
        case SessionIdRole:
            return session.id;
        case IsActiveRole:
            return session.isRecording;
        case EntryCountRole:
            return static_cast<qulonglong>(0);
        case SignalsListRole: {
            switch (session.type)
            {
                case DBC_BASED: {
                    QStringList list;
                    for (const auto& [id, _] : session.selectedSignals)
                        list.push_back(QString("0x") + QString::number(id, 16));
                    return list;
                }
                case RAW:
                    return QStringList{QString("Raw")};
                default:
                    return QVariant();
            }
        }
        default:
            return QVariant();
    }
}

QVariant LoggingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();

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
}

const LogSession* LoggingModel::getSession(const QString& sessionId) const
{
    for (const auto& session : m_sessions)
        if (session.id == sessionId) return &session;
    return nullptr;
}

QString LoggingModel::sessionIdAt(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size())) return QString();
    return m_sessions[index.row()].id;
}

[[nodiscard]] bool LoggingModel::isRecording() const
{
    return m_activeSessionIndex != -1;
}

QString LoggingModel::getCurrentSessionId() const
{
    if (m_activeSessionIndex >= 0 && m_activeSessionIndex < static_cast<int>(m_sessions.size()))
        return m_sessions[m_activeSessionIndex].id;
    return QString();
}

QString LoggingModel::getMessageName(uint16_t messageId) const
{
    if (!m_currentDbc.has_value())
        return QString("0x%1").arg(QString::number(messageId, 16).toUpper(), 3, QChar('0'));

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
        if (msgDef.messageId == messageId) return QString::fromStdString(msgDef.messageName);

    return QString("UNKNOWN_0x%1").arg(QString::number(messageId, 16).toUpper(), 3, QChar('0'));
}

QString LoggingModel::getSignalUnit(uint16_t messageId, const QString& signalName) const
{
    if (!m_currentDbc.has_value()) return QString();

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
    {
        if (msgDef.messageId != messageId) continue;
        for (const auto& sigDef : msgDef.signalDescriptions)
            if (QString::fromStdString(sigDef.signalName) == signalName)
                return QString::fromStdString(sigDef.unit);
    }
    return QString();
}

QStringList LoggingModel::getSelectedSignalsForMessage(uint16_t messageId) const
{
    if (m_activeSessionIndex < 0) return QStringList();
    const auto& activeSession = m_sessions[m_activeSessionIndex];
    const auto it = activeSession.selectedSignals.find(static_cast<uint32_t>(messageId));
    if (it != activeSession.selectedSignals.end()) return it->second;
    return QStringList();
}

void LoggingModel::updateDbcConfig(const Core::DbcConfig& config)
{
    stopActiveSession();
    m_currentDbc = config;
}

void LoggingModel::startNewDbcLogSession(const std::map<uint32_t, QStringList>& selectedSignals)
{
    if (isRecording()) stopActiveSession();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    LogSession newSession;
    newSession.id = QString::number(QDateTime::currentMSecsSinceEpoch());
    newSession.startDateTime = QDateTime::currentDateTime();
    newSession.duration = "00:00:00";
    newSession.isRecording = true;
    newSession.selectedSignals = selectedSignals;
    newSession.type = DBC_BASED;

    std::vector<CanStream::MessageInfo> schema;
    for (const auto& [msgId, sigNames] : selectedSignals)
    {
        CanStream::MessageInfo msgInfo;
        msgInfo.msgId = static_cast<uint16_t>(msgId);
        msgInfo.name = getMessageName(static_cast<uint16_t>(msgId)).toStdString();
        for (const auto& sig : sigNames)
        {
            CanStream::SignalInfo sigInfo;
            sigInfo.name = sig.toStdString();
            sigInfo.unit = getSignalUnit(static_cast<uint16_t>(msgId), sig).toStdString();
            msgInfo.signalList.push_back(std::move(sigInfo));
        }
        schema.push_back(std::move(msgInfo));
    }

    std::filesystem::create_directories("logs");
    auto writer = std::make_unique<CanStream::Mdf4Writer>(
        sessionFilePath(newSession.id).toStdString(), newSession.id.toULongLong(), schema);
    newSession.worker = std::make_unique<LoggingWorker>(std::move(writer));
    newSession.worker->start();
    m_dbcWorker.store(newSession.worker.get(), std::memory_order_release);

    m_sessions.push_back(std::move(newSession));
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();
}

void LoggingModel::startNewRawLogsSession()
{
    if (isRecording()) stopActiveSession();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    LogSession newSession;
    newSession.id = QString::number(QDateTime::currentMSecsSinceEpoch());
    newSession.startDateTime = QDateTime::currentDateTime();
    newSession.duration = "00:00:00";
    newSession.isRecording = true;
    newSession.type = RAW;

    std::filesystem::create_directories("logs");
    auto writer = std::make_unique<CanStream::Mdf4Writer>(
        sessionFilePath(newSession.id).toStdString(), newSession.id.toULongLong());
    newSession.worker = std::make_unique<LoggingWorker>(std::move(writer));
    newSession.worker->start();
    m_rawWorker.store(newSession.worker.get(), std::memory_order_release);

    m_sessions.push_back(std::move(newSession));
    m_activeSessionIndex = static_cast<int>(m_sessions.size()) - 1;
    endInsertRows();
}

void LoggingModel::stopActiveSession()
{
    if (!isRecording()) return;

    // Clear atomic pointers first so the event broker thread stops posting
    LoggingWorker* worker = m_rawWorker.exchange(nullptr, std::memory_order_acq_rel);
    if (!worker) worker = m_dbcWorker.exchange(nullptr, std::memory_order_acq_rel);

    m_sessions[m_activeSessionIndex].isRecording = false;
    m_activeSessionIndex = -1;

    // Drain remaining messages, close the file, and join the worker thread
    if (worker) worker->stop();

    updateActiveDuration();
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void LoggingModel::updateActiveDuration()
{
    if (!isRecording()) return;

    LogSession& activeSession = m_sessions[m_activeSessionIndex];
    const qint64 seconds = activeSession.startDateTime.secsTo(QDateTime::currentDateTime());
    activeSession.duration = QTime(0, 0).addSecs(static_cast<int>(seconds)).toString("hh:mm:ss");

    const QModelIndex durationIndex = index(m_activeSessionIndex, Col_Duration);
    emit dataChanged(durationIndex, durationIndex);
}

void LoggingModel::onDbcMessageReceived(const Core::DbcCanMessage& message)
{
    auto* w = m_dbcWorker.load(std::memory_order_acquire);
    if (w) w->post(message);
}

void LoggingModel::onRawMessageReceived(const Core::RawCanMessage& message)
{
    auto* w = m_rawWorker.load(std::memory_order_acquire);
    if (w) w->post(message);
}

// static
QString LoggingModel::sessionFilePath(const QString& sessionId)
{
    return QString("logs/session_%1.mf4").arg(sessionId);
}

void LoggingModel::onExportRequested(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_sessions.size())) return;

    const QString filePath = QFileDialog::getSaveFileName(
        nullptr, "Export Log", "log.csv", "CSV Files (*.csv);;MDF4 Files (*.mf4 *.mdf)");

    if (filePath.isEmpty()) return;

    const CanStream::ExportType type = filePath.endsWith(".csv", Qt::CaseInsensitive)
                                           ? CanStream::ExportType::Csv
                                           : CanStream::ExportType::Mdf4;

    CanStream::CanConverter converter(sessionFilePath(m_sessions[index.row()].id).toStdString(),
                                      filePath.toStdString(), type);
    (void)converter.convert();
}

}  // namespace Logging