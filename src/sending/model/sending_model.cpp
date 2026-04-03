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

#include "sending_model.hpp"

#include <algorithm>
#include <sstream>

#include "sending/constants.hpp"

namespace Sending {

SendingModel::SendingModel(QObject* parent) : QAbstractItemModel(parent), m_currentDbc(std::nullopt)
{
}

auto SendingModel::makeSignalKey(const uint16_t messageId,
                                 const std::string& signalName) -> std::string
{
    return std::to_string(messageId) + ':' + signalName;
}

auto SendingModel::index(const int row, const int column,
                         const QModelIndex& parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    if (!parent.isValid())
    {
        // Root level: messages
        if (m_currentDbc.has_value() &&
            row < static_cast<int>(m_currentDbc->messageDefinitions.size()))
        {
            return createIndex(row, column, nullptr);
        }
    } else
    {
        // Child level: signals under a message
        if (m_currentDbc.has_value())
        {
            auto it = m_currentDbc->messageDefinitions.begin();
            std::advance(it, parent.row());
            if (it != m_currentDbc->messageDefinitions.end() &&
                row < static_cast<int>(it->signalDescriptions.size()))
            {
                // Use parent row as internal id to identify which message this signal belongs to
                return createIndex(row, column, reinterpret_cast<void*>(parent.row() + 1));
            }
        }
    }

    return {};
}

auto SendingModel::parent(const QModelIndex& child) const -> QModelIndex
{
    if (!child.isValid())
    {
        return {};
    }

    // If internal pointer is null, this is a message (root level item)
    if (child.internalPointer() == nullptr)
    {
        return {};
    }

    // Otherwise, this is a signal - return the parent message index
    int messageRow = static_cast<int>(reinterpret_cast<quintptr>(child.internalPointer())) - 1;
    return createIndex(messageRow, 0, nullptr);
}

auto SendingModel::rowCount(const QModelIndex& parent) const -> int
{
    if (!m_currentDbc.has_value())
    {
        return 0;
    }

    if (!parent.isValid())
    {
        // Root level: return message count
        return static_cast<int>(m_currentDbc->messageDefinitions.size());
    }

    // If parent is a message (no internal pointer), return signal count
    if (parent.internalPointer() == nullptr)
    {
        auto it = m_currentDbc->messageDefinitions.begin();
        std::advance(it, parent.row());
        if (it != m_currentDbc->messageDefinitions.end())
        {
            return static_cast<int>(it->signalDescriptions.size());
        }
    }

    // Signals have no children
    return 0;
}

auto SendingModel::columnCount(const QModelIndex& /*parent*/) const -> int
{
    return 1;
}

auto SendingModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid())
    {
        return {};
    }

    // Handle global roles (not tied to specific index)
    switch (role)
    {
        case Role_ActiveMode:
            return static_cast<int>(m_currentMode);
        case Role_IsCyclicEnabled:
            return m_cyclicState.isEnabled;
        case Role_CycleIntervalMs:
            return m_cyclicState.intervalMs;
        case Role_IsCurrentlySending:
            return m_cyclicState.isSending;
        default:
            break;
    }

    if (!m_currentDbc.has_value())
    {
        return {};
    }

    // Check if this is a message or signal

    if (index.internalPointer() == nullptr)
    {
        // Message level
        auto it = m_currentDbc->messageDefinitions.begin();
        std::advance(it, index.row());
        if (it == m_currentDbc->messageDefinitions.end())
        {
            return {};
        }

        switch (role)
        {
            case Qt::DisplayRole:
                return QString::fromStdString(it->messageName);
            case Role_CanId:
                return it->messageId;
            default:
                return {};
        }
    } else
    {
        // Signal level
        int messageRow = static_cast<int>(reinterpret_cast<quintptr>(index.internalPointer())) - 1;
        auto msgIt = m_currentDbc->messageDefinitions.begin();
        std::advance(msgIt, messageRow);
        if (msgIt == m_currentDbc->messageDefinitions.end())
        {
            return {};
        }

        auto sigIt = msgIt->signalDescriptions.begin();
        std::advance(sigIt, index.row());
        if (sigIt == msgIt->signalDescriptions.end())
        {
            return {};
        }

        switch (role)
        {
            case Qt::DisplayRole:
                return QString::fromStdString(sigIt->signalName);
            case Role_SignalValue: {
                const std::string key = makeSignalKey(msgIt->messageId, sigIt->signalName);
                auto valIt = m_dynamicSignalValues.find(key);
                if (valIt != m_dynamicSignalValues.end())
                {
                    return valIt->second;
                }
                return sigIt->minimum;  // Default to minimum
            }
            default:
                return {};
        }
    }
}

auto SendingModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    switch (role)
    {
        case Role_ActiveMode:
            m_currentMode = static_cast<Mode>(value.toInt());
            emit dataChanged(index, index, {role});
            return true;

        case Role_IsCyclicEnabled:
            m_cyclicState.isEnabled = value.toBool();
            emit dataChanged(index, index, {role});
            return true;

        case Role_CycleIntervalMs: {
            int interval = value.toInt();
            if (interval < Constants::MIN_CYCLE_INTERVAL_MS)
            {
                interval = Constants::MIN_CYCLE_INTERVAL_MS;
            }
            if (interval > Constants::MAX_CYCLE_INTERVAL_MS)
            {
                interval = Constants::MAX_CYCLE_INTERVAL_MS;
            }
            m_cyclicState.intervalMs = interval;
            emit dataChanged(index, index, {role});
            return true;
        }

        case Role_SignalValue:
            if (index.isValid() && index.internalPointer() != nullptr && m_currentDbc.has_value())
            {
                // This is a signal index
                const int messageRow =
                    static_cast<int>(reinterpret_cast<quintptr>(index.internalPointer())) - 1;
                auto msgIt = m_currentDbc->messageDefinitions.begin();
                std::advance(msgIt, messageRow);
                if (msgIt != m_currentDbc->messageDefinitions.end())
                {
                    auto sigIt = msgIt->signalDescriptions.begin();
                    std::advance(sigIt, index.row());
                    if (sigIt != msgIt->signalDescriptions.end())
                    {
                        double val = value.toDouble();
                        // Clamp to valid range
                        if (val < sigIt->minimum)
                        {
                            val = sigIt->minimum;
                        }
                        if (val > sigIt->maximum)
                        {
                            val = sigIt->maximum;
                        }
                        const std::string key = makeSignalKey(msgIt->messageId, sigIt->signalName);
                        m_dynamicSignalValues[key] = val;
                        emit dataChanged(index, index, {role});
                        return true;
                    }
                }
            }
            return false;

        default:
            return false;
    }
}

void SendingModel::updateDbcConfig(const Core::DbcConfig& config)
{
    beginResetModel();
    // Make a copy of the config (owned by this model)
    m_currentDbc = config;

    // Clear selection, values, and evaluators
    m_dynamicSignalValues.clear();
    m_selectedSignalNames.clear();
    {
        std::lock_guard lock(m_evalMutex);
        m_signalEvaluators.clear();
    }

    // Initialize signal values to their minimums
    for (const auto& msg : m_currentDbc->messageDefinitions)
    {
        for (const auto& sig : msg.signalDescriptions)
        {
            const std::string key = makeSignalKey(msg.messageId, sig.signalName);
            m_dynamicSignalValues[key] = sig.minimum;
        }
    }

    endResetModel();
}

void SendingModel::setTransmissionStatus(const bool isActive)
{
    if (m_cyclicState.isSending == isActive)
    {
        return;
    }
    m_cyclicState.isSending = isActive;
    emit dataChanged(QModelIndex(), QModelIndex(), {Role_IsCurrentlySending});
}

void SendingModel::transmitCurrent()
{
    forEachPendingMessage([this](const Core::RawCanMessage& msg) { emit requestSendRaw("", msg); },
                          [this](const Core::DbcCanMessage& msg) { emit requestSendDbc("", msg); });
}

void SendingModel::forEachPendingMessage(
    const std::function<void(const Core::RawCanMessage&)>& rawHandler,
    const std::function<void(const Core::DbcCanMessage&)>& dbcHandler) const
{
    if (m_currentMode == Mode::Raw)
    {
        Core::RawCanMessage message{};
        message.messageId = static_cast<uint16_t>(m_rawState.id & Constants::MAX_CAN_ID);
        message.dlc = m_rawState.dlc;

        for (size_t i = 0; i < Constants::MAX_CAN_DLC && i < m_rawState.data.size(); ++i)
        {
            message.data[i] = static_cast<char>(m_rawState.data[i]);
        }

        if (rawHandler)
        {
            rawHandler(message);
        }
    } else
    {
        // DBC mode - send messages that have any selected signals
        if (!m_currentDbc.has_value())
        {
            return;
        }

        for (const auto& msgDef : m_currentDbc->messageDefinitions)
        {
            Core::DbcCanMessage message{};
            message.messageId = static_cast<uint16_t>(msgDef.messageId & Constants::MAX_CAN_ID);

            // Populate only selected signal values
            for (const auto& sigDef : msgDef.signalDescriptions)
            {
                // Only include signals that are selected
                if (!isSignalSelected(msgDef.messageId, sigDef.signalName))
                {
                    continue;
                }

                Core::DbcCanSignal signal;
                signal.name = sigDef.signalName;
                signal.value = sigDef.minimum;

                const std::string key = makeSignalKey(msgDef.messageId, sigDef.signalName);
                bool hasEvaluator = false;
                {
                    std::lock_guard lock(m_evalMutex);
                    if (auto evIt = m_signalEvaluators.find(key); evIt != m_signalEvaluators.end())
                    {
                        signal.value = evIt->second();
                        hasEvaluator = true;
                    }
                }
                if (!hasEvaluator)
                {
                    if (auto valIt = m_dynamicSignalValues.find(key);
                        valIt != m_dynamicSignalValues.end())
                    {
                        signal.value = valIt->second;
                    }
                }

                signal.value = std::clamp(signal.value, sigDef.minimum, sigDef.maximum);

                message.signalValues.push_back(signal);
            }

            // Only send if this message has at least one selected signal
            if (!message.signalValues.empty() && dbcHandler)
            {
                dbcHandler(message);
            }
        }
    }
}

void SendingModel::setMessageSelected(const uint16_t messageId, const bool selected)
{
    // Select/deselect ALL signals of this message (convenience for "select all" checkbox)
    if (!m_currentDbc.has_value())
    {
        return;
    }

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
    {
        if (msgDef.messageId == messageId)
        {
            for (const auto& sigDef : msgDef.signalDescriptions)
            {
                setSignalSelected(messageId, sigDef.signalName, selected);
            }
            break;
        }
    }
}

auto SendingModel::isMessageSelected(const uint16_t messageId) const -> bool
{
    // Returns true only if ALL signals of this message are selected
    if (!m_currentDbc.has_value())
    {
        return false;
    }

    for (const auto& msgDef : m_currentDbc->messageDefinitions)
    {
        if (msgDef.messageId == messageId)
        {
            if (msgDef.signalDescriptions.empty())
            {
                return false;
            }
            for (const auto& sigDef : msgDef.signalDescriptions)
            {
                if (!isSignalSelected(messageId, sigDef.signalName))
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void SendingModel::setSignalSelected(const uint16_t messageId, const std::string& signalName,
                                     const bool selected)
{
    const std::string key = makeSignalKey(messageId, signalName);
    if (selected)
    {
        m_selectedSignalNames.insert(key);
    } else
    {
        m_selectedSignalNames.erase(key);
    }
}

auto SendingModel::isSignalSelected(const uint16_t messageId,
                                    const std::string& signalName) const -> bool
{
    const std::string key = makeSignalKey(messageId, signalName);
    return m_selectedSignalNames.contains(key);
}

void SendingModel::setSignalValue(const uint16_t messageId, const std::string& signalName,
                                  const double value)
{
    double clamped = value;

    if (m_currentDbc.has_value())
    {
        for (const auto& msg : m_currentDbc->messageDefinitions)
        {
            if (msg.messageId != messageId)
            {
                continue;
            }
            for (const auto& sig : msg.signalDescriptions)
            {
                if (sig.signalName == signalName)
                {
                    clamped = std::clamp(value, sig.minimum, sig.maximum);
                    break;
                }
            }
            break;
        }
    }

    const std::string key = makeSignalKey(messageId, signalName);
    m_dynamicSignalValues[key] = clamped;
}

void SendingModel::setSignalEvaluator(const uint16_t messageId, const std::string& signalName,
                                      SignalEvaluator evaluator)
{
    const std::string key = makeSignalKey(messageId, signalName);
    std::lock_guard lock(m_evalMutex);
    if (evaluator)
        m_signalEvaluators[key] = std::move(evaluator);
    else
        m_signalEvaluators.erase(key);
}

void SendingModel::clearEvaluators()
{
    std::lock_guard lock(m_evalMutex);
    m_signalEvaluators.clear();
}

void SendingModel::setRawCanId(const uint16_t canId)
{
    m_rawState.id = canId & Constants::MAX_CAN_ID;
}

void SendingModel::setRawData(const std::vector<uint8_t>& data)
{
    m_rawState.data.clear();
    m_rawState.dlc = 0;
    for (size_t i = 0; i < Constants::MAX_CAN_DLC && i < data.size(); ++i)
    {
        m_rawState.data.push_back(data[i]);
        m_rawState.dlc++;
    }
    // Pad with zeros
    while (m_rawState.data.size() < Constants::MAX_CAN_DLC)
    {
        m_rawState.data.push_back(0);
    }
}

}  // namespace Sending
