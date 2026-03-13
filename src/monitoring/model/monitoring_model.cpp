#include "monitoring_model.hpp"

#include <memory>
#include <mutex>

#include "monitoring/constants.hpp"

namespace Monitoring {

MonitoringModel::MonitoringModel() : QAbstractItemModel(nullptr)
{
    _execute = true;
    deleteOldData = true;
    messageValues = std::make_unique<std::array<MessageTimestamp, 2048>>();
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
        if (m_currentDbc->messageDefinitions.size() > row)
        {
            return createIndex(row, column, nullptr);
        }
    } else
    {
        // signal level
        auto it = m_currentDbc->messageDefinitions.begin();
        std::advance(it, parent.row());
        if (row < it->signalDescriptions.size())
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
        return static_cast<int>(m_currentDbc->messageDefinitions.size());
    }
    if (parent.internalPointer() == nullptr)
    {  // parent is message -> signal count for given message
        if (parent.row() >= static_cast<int>(m_currentDbc->messageDefinitions.size()))
        {
            return 0;
        }
        auto it = m_currentDbc->messageDefinitions.begin();
        std::advance(it, parent.row());
        return static_cast<int>(it->signalDescriptions.size());
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
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, index.row());
                if (it->messageId >= messageValues->size())
                {
                    return QVariant();
                }
                return messageValues->at(it->messageId).timestamps.size() == 0
                           ? QVariant()
                           : messageValues->at(it->messageId).timestamps.back();
            }
            case Role_ValueList: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, index.row());
                if (it->messageId >= messageValues->size())
                {
                    return QVariant();
                }
                return QVariant::fromValue(messageValues->at(it->messageId).timestamps);
            }
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
            case Role_LatestValue: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it->messageId >= messageValues->size())
                {
                    return QVariant();
                }
                return messageValues->at(it->messageId).signalValues.at(index.row()).size() == 0
                           ? QVariant()
                           : messageValues->at(it->messageId).signalValues.at(index.row()).back();
            }
            case Role_ValueList: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it->messageId >= messageValues->size())
                {
                    return QVariant();
                }
                return QVariant::fromValue(
                    messageValues->at(it->messageId).signalValues.at(index.row()));
            }
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
            case Role_Max: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    auto it2 = it->signalDescriptions.begin();
                    std::advance(it2, index.row());
                    if (it2 != it->signalDescriptions.end())
                    {
                        return it2->maximum;
                    }
                }
                return {};
            }
            case Role_Min: {
                auto it = m_currentDbc->messageDefinitions.begin();
                std::advance(it, messageRow);
                if (it != m_currentDbc->messageDefinitions.end())
                {
                    auto it2 = it->signalDescriptions.begin();
                    std::advance(it2, index.row());
                    if (it2 != it->signalDescriptions.end())
                    {
                        return it2->minimum;
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
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    if (!m_currentDbc.has_value() || !deleteOldData || message.messageId >= messageValues->size())
    {
        return;
    }

    int j = 0;
    for (auto& signalName : messageValues->at(message.messageId).signalNames)
    {
        bool valueExists = false;
        for (auto it3 = message.signalValues.begin(); it3 != message.signalValues.end(); ++it3)
        {
            if (signalName == QString::fromStdString(it3->name))
            {
                messageValues->at(message.messageId).signalValues.at(j).push_back(it3->value);
                valueExists = true;
                break;
            }
        }
        if (!valueExists)
        {
            messageValues->at(message.messageId).signalValues.at(j).push_back(NAN);
        }
        j++;
    }
    messageValues->at(message.messageId).timestamps.push_back(message.receiveTime.count());
}

void MonitoringModel::onDbcChange(const Core::DbcConfig& config)
{
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    m_currentDbc = config;
    messageValues = std::make_unique<std::array<MessageTimestamp, 2048>>();
    int row = 0;
    for (auto& messageDefinition : m_currentDbc->messageDefinitions)
    {
        std::vector<QList<double>> signalValues;
        QList<QString> signalNames;
        for (auto& signalDescription : messageDefinition.signalDescriptions)
        {
            signalValues.emplace_back();
            signalNames.push_back(QString::fromStdString(signalDescription.signalName));
        }
        if (messageDefinition.messageId >= messageValues->size())
        {
            continue;
        }
        messageValues->at(messageDefinition.messageId) = MessageTimestamp{
            .timestamps = {}, .signalValues = signalValues, .signalNames = signalNames};
        row++;
    }
}

void MonitoringModel::eraseOldData()
{
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
    for (auto& messageValue : *messageValues)
    {
        while (!messageValue.timestamps.empty() &&
               messageValue.timestamps.front() + Constants::HOLDING_SECONDS_IN_MODEL * 1000 <
                   milliseconds)
        {
            for (auto& j : messageValue.signalValues)
            {
                if (!j.empty())
                {
                    j.pop_front();
                }
            }
            messageValue.timestamps.pop_front();
        }
    }
}

}  // namespace Monitoring