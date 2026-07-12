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

#include "monitoring_model.hpp"

#include <chrono>
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

    auto& entry = messageValues->at(message.messageId);
    const auto timestampNs = static_cast<qreal>(message.receiveTime.count());

    if (entry.windowStartNs < 0)
    {
        entry.windowStartNs = timestampNs;
    }

    const qreal windowSizeNs = Constants::AGGREGATION_WINDOW_MS * 1'000'000.0;
    if (timestampNs - entry.windowStartNs >= windowSizeNs)
    {
        flushWindow(entry, timestampNs);
    }

    int j = 0;
    for (auto& signalName : entry.signalNames)
    {
        for (const auto& [name, value] : message.signalValues)
        {
            if (signalName == QString::fromStdString(name))
            {
                entry.windowSum[j] += value;
                entry.windowCount[j] += 1;
                break;
            }
        }
        j++;
    }
}

void MonitoringModel::flushWindow(MessageTimestamp& entry, const qreal newWindowStartNs)
{
    entry.timestamps.push_back(entry.windowStartNs);
    for (std::size_t k = 0; k < entry.signalValues.size(); ++k)
    {
        entry.signalValues[k].push_back(
            entry.windowCount[k] > 0 ? entry.windowSum[k] / entry.windowCount[k] : NAN);
        entry.windowSum[k] = 0.0;
        entry.windowCount[k] = 0;
    }
    entry.windowStartNs = newWindowStartNs;
}

void MonitoringModel::flushStaleWindows()
{
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    if (!m_currentDbc.has_value() || !deleteOldData)
    {
        return;
    }

    const auto nowNs =
        static_cast<qreal>(std::chrono::steady_clock::now().time_since_epoch().count());
    const qreal windowSizeNs = Constants::AGGREGATION_WINDOW_MS * 1'000'000.0;

    for (auto& entry : *messageValues)
    {
        if (entry.windowStartNs >= 0 && nowNs - entry.windowStartNs >= windowSizeNs)
        {
            flushWindow(entry, -1);
        }
    }
}

void MonitoringModel::onDbcChange(const Core::DbcConfig& config)
{
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    m_currentDbc = config;
    messageValues = std::make_unique<std::array<MessageTimestamp, 2048>>();
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
        messageValues->at(messageDefinition.messageId) =
            MessageTimestamp{.timestamps = {},
                             .signalValues = signalValues,
                             .signalNames = signalNames,
                             .windowStartNs = -1,
                             .windowSum = std::vector<double>(signalValues.size(), 0.0),
                             .windowCount = std::vector<int>(signalValues.size(), 0)};
    }
}

void MonitoringModel::eraseOldData()
{
    std::scoped_lock<std::mutex> lock(m_dataMutex);
    const auto nowNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::steady_clock::now().time_since_epoch())
                           .count();
    for (auto& messageValue : *messageValues)
    {
        while (!messageValue.timestamps.empty() &&
               messageValue.timestamps.front() +
                       Constants::HOLDING_SECONDS_IN_MODEL * 1'000'000'000LL <
                   nowNs)
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