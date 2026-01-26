#include "sending_model.hpp"

#include "sending/constants.hpp"

namespace Sending {

SendingModel::SendingModel(QObject* parent)
    : QAbstractItemModel(parent), m_currentDbc(nullptr), m_cyclicTimer(new QTimer(this))
{
    connect(m_cyclicTimer, &QTimer::timeout, this, &SendingModel::onCyclicTimerTimeout);
}

auto SendingModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    if (!parent.isValid())
    {
        // Root level: messages
        if (m_currentDbc && row < static_cast<int>(m_currentDbc->messageDefinitions.size()))
        {
            return createIndex(row, column, nullptr);
        }
    } else
    {
        // Child level: signals under a message
        if (m_currentDbc)
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
    if (!m_currentDbc)
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

    if (!m_currentDbc)
    {
        return {};
    }

    // Check if this is a message or signal
    bool isMessage = (index.internalPointer() == nullptr);

    if (isMessage)
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
                auto valIt = m_dynamicSignalValues.find(sigIt->signalName);
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
            updateTimerState();
            emit dataChanged(index, index, {role});
            return true;

        case Role_CycleIntervalMs: {
            int interval = value.toInt();
            // Enforce minimum interval (prevent system overload)
            if (interval < Constants::MIN_CYCLE_INTERVAL_MS)
            {
                interval = Constants::MIN_CYCLE_INTERVAL_MS;
            }
            // Enforce maximum interval
            if (interval > Constants::MAX_CYCLE_INTERVAL_MS)
            {
                interval = Constants::MAX_CYCLE_INTERVAL_MS;
            }
            m_cyclicState.intervalMs = interval;
            updateTimerState();
            emit dataChanged(index, index, {role});
            return true;
        }

        case Role_SignalValue:
            if (index.isValid() && index.internalPointer() != nullptr)
            {
                // This is a signal index
                int messageRow =
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
                        m_dynamicSignalValues[sigIt->signalName] = val;
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
    // Store a copy of the config
    static Core::DbcConfig storedConfig;
    storedConfig = config;
    m_currentDbc = &storedConfig;

    // Initialize signal values to their minimums
    m_dynamicSignalValues.clear();
    for (const auto& msg : m_currentDbc->messageDefinitions)
    {
        for (const auto& sig : msg.signalDescriptions)
        {
            m_dynamicSignalValues[sig.signalName] = sig.minimum;
        }
    }

    endResetModel();
}

void SendingModel::setTransmissionStatus(bool isActive)
{
    if (m_cyclicState.isSending != isActive)
    {
        m_cyclicState.isSending = isActive;
        if (!isActive)
        {
            m_cyclicTimer->stop();
        }
        emit dataChanged(QModelIndex(), QModelIndex(), {Role_IsCurrentlySending});
    }
}

void SendingModel::transmitCurrent()
{
    if (m_currentMode == Mode::Raw)
    {
        Core::RawCanMessage message;
        message.receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        message.messageId = static_cast<char>(m_rawState.id & 0xFF);

        for (size_t i = 0; i < 8 && i < m_rawState.data.size(); ++i)
        {
            message.data[i] = static_cast<char>(m_rawState.data[i]);
        }

        emit requestSendRaw("", message);
    } else
    {
        // DBC mode - send all selected messages
        if (!m_currentDbc)
        {
            return;
        }

        for (uint32_t msgId : m_selectedMessageIds)
        {
            // Find the message definition
            for (const auto& msgDef : m_currentDbc->messageDefinitions)
            {
                if (msgDef.messageId == msgId)
                {
                    Core::DbcCanMessage message;
                    message.receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch());
                    message.messageId = static_cast<char>(msgId & 0xFF);

                    // Populate signal values
                    for (const auto& sigDef : msgDef.signalDescriptions)
                    {
                        Core::DbcCanSignal signal;
                        signal.name = sigDef.signalName;

                        auto valIt = m_dynamicSignalValues.find(sigDef.signalName);
                        if (valIt != m_dynamicSignalValues.end())
                        {
                            signal.value = valIt->second;
                        } else
                        {
                            signal.value = sigDef.minimum;
                        }

                        message.signalValues.push_back(signal);
                    }

                    emit requestSendDbc("", message);
                    break;
                }
            }
        }
    }
}

void SendingModel::onCyclicTimerTimeout()
{
    if (m_cyclicState.isEnabled && m_cyclicState.isSending)
    {
        transmitCurrent();
    }
}

void SendingModel::updateTimerState()
{
    if (m_cyclicState.isEnabled && m_cyclicState.isSending)
    {
        m_cyclicTimer->start(m_cyclicState.intervalMs);
    } else
    {
        m_cyclicTimer->stop();
    }
}

}  // namespace Sending
