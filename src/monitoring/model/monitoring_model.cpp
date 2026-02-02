#include "monitoring_model.hpp"

#include "monitoring/constants.hpp"

namespace Monitoring {

MonitoringModel::MonitoringModel() : QAbstractItemModel(nullptr)
{
    _execute = true;
    deleteOldData = true;
    message_check_thread = std::thread([this]() {
        long long lastExecution = 0;
        while (_execute)
        {
            const auto millisecondsBeforeErase =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            if (millisecondsBeforeErase > lastExecution + 50 && deleteOldData)
            {
                lastExecution = millisecondsBeforeErase;

                eraseOldData();
            }
            const auto millisecondsAfterErase =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            if (millisecondsAfterErase < lastExecution + 50)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(lastExecution + 50 - millisecondsAfterErase));
            }
        }
    });
}

// --- Tree Navigation ---

auto MonitoringModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent) || !m_currentDbc.has_value())
    {
        return {};
    }
    if (!parent.isValid())
    {  // message level
        if (row < static_cast<int>(messageValues.size()))
        {
            return createIndex(row, column, nullptr);
        }
    } else
    {
        // signal level
        if (row < messageValues.at(parent.row()).signalValues.size())
        {
            return createIndex(row, column, reinterpret_cast<void*>(parent.row() + 1));
        }
    }
    return {};
}

auto MonitoringModel::parent(const QModelIndex& child) const -> QModelIndex
{
    if (!child.isValid()) return {};

    // If internal pointer is null, this is a message (root level item)
    if (child.internalPointer() == nullptr)
    {
        return {};
    }

    const int messageRow =
        static_cast<int>(reinterpret_cast<quintptr>(child.internalPointer())) - 1;
    return createIndex(messageRow, 0, nullptr);
}

auto MonitoringModel::rowCount(const QModelIndex& parent) const -> int
{
    if (!m_currentDbc.has_value())
    {
        return 0;
    }
    if (!parent.isValid())
    {  // parent is root -> message count
        return static_cast<int>(messageValues.size());
    }
    if (parent.internalPointer() == nullptr)
    {  // parent is message -> signal count for given message
        if (parent.row() >= static_cast<int>(messageValues.size()))
        {
            return 0;
        }
        return static_cast<int>(messageValues.at(parent.row()).signalValues.size());
    }
    return 0;
}

auto MonitoringModel::columnCount(const QModelIndex& /*parent*/) const -> int
{
    return 1;  // ID and Name combined in one column for simplicity, or 2 if you want split
}

// --- Data Display ---

auto MonitoringModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid() || !m_currentDbc.has_value() || index.row() >= rowCount(index.parent()))
        return {};
    if (index.internalPointer() == nullptr)
    {  // message level
        switch (role)
        {
            case Qt::DisplayRole:
            case Role_Name: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, index.row());
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    return QString(it->messageName.data());
                }
                return {};
            }
            case Role_ID: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, index.row());
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    return it->messageId;
                }
                return {};
            }
            case Role_LatestValue: {
            }
                return messageValues.at(index.row()).timestamps.size() == 0
                           ? QVariant()
                           : messageValues.at(index.row()).timestamps.back();
            case Role_ValueList:
                return QVariant::fromValue(messageValues.at(index.row()).timestamps);
            case Role_Unit:
                return {};
            default:
                return {};
        }
    } else
    {  // signal level
        const int messageRow =
            static_cast<int>(reinterpret_cast<quintptr>(index.internalPointer())) - 1;
        switch (role)
        {
            case Qt::DisplayRole:
            case Role_Name: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    auto it2 = it->signalDescriptions.begin();
                    std::advance(it2, index.row());
                    if (it2 != it->signalDescriptions.end())
                    {
                        return QString(it2->signalName.data());
                    }
                }
                return {};
            }
            case Role_ID:
                return {};
            case Role_LatestValue:
                return messageValues.at(messageRow).signalValues.at(index.row()).size() == 0
                           ? QVariant()
                           : messageValues.at(index.parent().row())
                                 .signalValues.at(index.row())
                                 .back();
            case Role_ValueList:
                return QVariant::fromValue(
                    messageValues.at(messageRow).signalValues.at(index.row()));
            case Role_Unit: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    auto it2 = it->signalDescriptions.begin();
                    std::advance(it2, index.row());
                    if (it2 != it->signalDescriptions.end())
                    {
                        return QString(it2->unit.data());
                    }
                }
                return {};
            }
            default:
                return {};
        }
    }
}

void MonitoringModel::onIncomingDbcFrame(const Core::DbcCanMessage& message)
{
    if (!m_currentDbc.has_value() || !deleteOldData)
    {
        return;
    }
    int i = 0;
    for (auto it = m_currentDbc->messageDefinitions.begin();
         it != m_currentDbc->messageDefinitions.end(); ++it)
    {
        if (it->messageId == message.messageId)
        {
            int j = 0;
            for (auto it2 = it->signalDescriptions.begin(); it2 != it->signalDescriptions.end();
                 ++it2)
            {
                bool valueExists = false;
                for (auto it3 = message.signalValues.begin(); it3 != message.signalValues.end();
                     ++it3)
                {
                    if (it3->name == it2->signalName)
                    {
                        messageValues.at(i).signalValues.at(j).push_back(it3->value);
                        valueExists = true;
                        break;
                    }
                }
                if (!valueExists)
                {
                    messageValues.at(i).signalValues.at(j).push_back(NAN);
                }
                j++;
            }
            break;
        }
        i++;
    }
}

void MonitoringModel::onDbcChange(const Core::DbcConfig& config)
{
    m_currentDbc = config;
    while (!messageValues.empty())
    {
        messageValues.pop_back();
    }
    for (auto& messageDefinition : m_currentDbc->messageDefinitions)
    {
        std::vector<QList<double>> signalValues;
        for (int i = 0; i < messageDefinition.signalDescriptions.size(); i++)
        {
            signalValues.emplace_back();
        }
        messageValues.push_back(MessageTimestamp{.timestamps = {}, .signalValues = signalValues});
    }
}

void MonitoringModel::eraseOldData()
{
    QTime currentTime = QTime::currentTime();
    for (auto& [timestamps, signalValues] : messageValues)
    {
        if (timestamps.isEmpty())
        {
            continue;
        }
        if (timestamps.begin()->msecsTo(
                QTime::currentTime().addSecs(Constants::HOLDING_SECONDS_IN_MODEL)) < 0)
        {
            for (auto& j : signalValues)
            {
                j.pop_front();
            }
            timestamps.pop_front();
        }
    }
}

}  // namespace Monitoring