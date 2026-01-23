#include "monitoring_model.hpp"

namespace Monitoring {

MonitoringModel::MonitoringModel() : QAbstractItemModel(nullptr) {}

// --- Tree Navigation ---

auto MonitoringModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent)) return {};

    // If no parent, we are looking for a Frame (Top Level)
    if (!parent.isValid())
    {
        return createIndex(row, column, (void*)&m_frames[row]);
    }

    // If parent is valid, it's a Frame, and we are looking for a Signal (Child Level)
    auto* parentFrame = static_cast<FrameNode*>(parent.internalPointer());
    if (parentFrame && row < parentFrame->allSignals.size())
    {
        return createIndex(row, column, (void*)&parentFrame->allSignals[row]);
    }

    return {};
}

auto MonitoringModel::parent(const QModelIndex& index) const -> QModelIndex
{
    if (!index.isValid()) return {};

    // Determine if this pointer belongs to a Signal or a Frame
    // We can check if it exists in the m_frames vector
    void* ptr = index.internalPointer();

    // Check if the pointer is one of our frames
    for (const auto& m_frame : m_frames)
    {
        if (ptr == &m_frame) return {};  // Frames have no parent
    }

    // If it's not a frame, it must be a signal. We find which frame owns it.
    for (int i = 0; i < m_frames.size(); ++i)
    {
        for (int j = 0; j < m_frames[i].allSignals.size(); ++j)
        {
            if (ptr == &m_frames[i].allSignals[j])
            {
                return createIndex(i, 0, (void*)&m_frames[i]);
            }
        }
    }

    return {};
}

auto MonitoringModel::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.column() > 0) return 0;

    // Root level: return number of frames
    if (!parent.isValid()) return m_frames.size();

    // Child level: return number of signals in that frame
    auto* frame = static_cast<FrameNode*>(parent.internalPointer());
    return frame ? frame->allSignals.size() : 0;
}

auto MonitoringModel::columnCount(const QModelIndex& /*parent*/) const -> int
{
    return 1;  // ID and Name combined in one column for simplicity, or 2 if you want split
}

// --- Data Display ---

auto MonitoringModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid()) return {};

    void* ptr = index.internalPointer();

    // Handle Frame Node
    for (const auto& frame : m_frames)
    {
        if (ptr == &frame)
        {
            if (role == Qt::DisplayRole)
                return QString("ID: 0x%1 - %2")
                    .arg(frame.message.messageId, 1)
                    // TODO
                    .arg(QString::fromStdString(""));
            if (role == Qt::CheckStateRole) return frame.checked;
            if (role == Qt::UserRole + 1) return frame.message.messageId;  // Helpful for your View!
            return {};
        }
    }

    // Handle Signal Node
    for (const auto& frame : m_frames)
    {
        for (const auto& sig : frame.allSignals)
        {
            if (ptr == &sig)
            {
                if (role == Qt::DisplayRole) return QString::fromStdString(sig.signal.name);
                if (role == Qt::CheckStateRole) return sig.checked;
                if (role == Qt::UserRole + 1)
                    return frame.message.messageId;  // Signal needs to know parent ID
                return {};
            }
        }
    }

    return {};
}

// --- Interaction (Checkboxes) ---

auto MonitoringModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

auto MonitoringModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    if (role == Qt::CheckStateRole)
    {
        void* ptr = index.internalPointer();
        auto newState = static_cast<Qt::CheckState>(value.toInt());

        // Find and update the node
        for (auto& frame : m_frames)
        {
            if (ptr == &frame)
            {
                frame.checked = newState;
                emit dataChanged(index, index, {role});
                return true;
            }
            for (auto& sig : frame.allSignals)
            {
                if (ptr == &sig)
                {
                    sig.checked = newState;
                    emit dataChanged(index, index, {role});
                    return true;
                }
            }
        }
    }
    return false;
}

// --- Incoming Data ---

void MonitoringModel::onIncomingDbcFrame(const Core::DbcCanMessage& message)
{
    // Check if frame already exists
    for (int i = 0; i < m_frames.size(); ++i)
    {
        if (m_frames[i].message.messageId == message.messageId)
        {
            m_frames[i].message = message;  // Update data
            // We don't update allSignals structure here assuming DBC is static
            emit dataChanged(this->index(i, 0, QModelIndex()), this->index(i, 0, QModelIndex()));
            return;
        }
    }

    // New Frame discovered!
    beginInsertRows(QModelIndex(), m_frames.size(), m_frames.size());
    FrameNode newNode;
    newNode.message = message;
    newNode.checked = Qt::Unchecked;

    for (const auto& sig : message.signalValues)
    {
        newNode.allSignals.append({.signal = sig, .checked = Qt::Unchecked});
    }

    m_frames.append(newNode);
    endInsertRows();
}

void MonitoringModel::onIncomingRawFrame(const Core::RawCanMessage& /*message*/)
{
    // Logic for raw frames if you want to show them in the tree
}

}  // namespace Monitoring