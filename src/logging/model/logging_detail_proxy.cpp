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

#include "logging_detail_proxy.hpp"

#include <QString>

#include "logging/constants.hpp"
#include "logging/model/logging_model.hpp"

namespace Logging {

LoggingDetailProxy::LoggingDetailProxy(QObject* parent)
    : QAbstractProxyModel(parent), m_buffer(std::vector<Core::RawCanMessage>{})
{
}

void LoggingDetailProxy::setSessionIndex(const QModelIndex& sessionIndex)
{
    const auto* model = qobject_cast<LoggingModel*>(sourceModel());
    if (!model || !sessionIndex.isValid()) return;

    m_sessionIndex = sessionIndex;

    const QString filePath = LoggingModel::sessionFilePath(model->sessionIdAt(sessionIndex));

    m_reader = std::make_unique<CanStream::Mdf4Reader>();
    if (!m_reader->open(filePath.toStdString()))
    {
        m_reader.reset();
        return;
    }

    m_fileType = m_reader->fileType();
    buildHeaders();
    loadPage(0);
}

auto LoggingDetailProxy::hasNextPage() const -> bool
{
    if (!m_reader) return false;
    return m_pageOffset + Constants::DETAIL_PAGE_SIZE < m_reader->recordCount();
}

auto LoggingDetailProxy::hasPrevPage() const -> bool
{
    return m_pageOffset > 0;
}

auto LoggingDetailProxy::currentPage() const -> int
{
    return static_cast<int>(m_pageOffset / Constants::DETAIL_PAGE_SIZE) + 1;
}

auto LoggingDetailProxy::pageCount() const -> int
{
    if (!m_reader || m_reader->recordCount() == 0) return 1;
    return static_cast<int>((m_reader->recordCount() + Constants::DETAIL_PAGE_SIZE - 1) /
                            Constants::DETAIL_PAGE_SIZE);
}

auto LoggingDetailProxy::totalRecordCount() const -> uint64_t
{
    if (!m_reader) return 0;
    return m_reader->recordCount();
}

void LoggingDetailProxy::loadPage(const uint64_t offset)
{
    if (!m_reader || !m_reader->seek(offset)) return;

    beginResetModel();
    m_pageOffset = offset;

    if (m_fileType == Core::CanFileType::Raw)
    {
        auto& buf = m_buffer.emplace<std::vector<Core::RawCanMessage>>();
        Core::RawCanMessage msg;
        while (static_cast<int>(buf.size()) < Constants::DETAIL_PAGE_SIZE && m_reader->read(msg))
            buf.push_back(msg);
    } else
    {
        auto& buf = m_buffer.emplace<std::vector<Core::DbcCanMessage>>();
        Core::DbcCanMessage msg;
        while (static_cast<int>(buf.size()) < Constants::DETAIL_PAGE_SIZE && m_reader->read(msg))
            buf.push_back(msg);
    }

    endResetModel();
}

auto LoggingDetailProxy::mapToSource(const QModelIndex& /*proxyIndex*/) const -> QModelIndex
{
    return m_sessionIndex;
}

auto LoggingDetailProxy::mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex
{
    if (sourceIndex == m_sessionIndex) return createIndex(0, 0);
    return {};
}

auto LoggingDetailProxy::index(const int row, const int column, const QModelIndex& parent) const
    -> QModelIndex
{
    if (parent.isValid()) return {};
    return createIndex(row, column);
}

auto LoggingDetailProxy::parent(const QModelIndex& /*child*/) const -> QModelIndex
{
    return {};
}

auto LoggingDetailProxy::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) return 0;
    return std::visit([](const auto& buf) { return static_cast<int>(buf.size()); }, m_buffer);
}

auto LoggingDetailProxy::columnCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) return 0;
    return m_headers.size();
}

auto LoggingDetailProxy::data(const QModelIndex& index, const int role) const -> QVariant
{
    if (!index.isValid()) return {};
    if (role == Qt::TextAlignmentRole) return static_cast<int>(Qt::AlignCenter);
    if (role != Qt::DisplayRole) return {};

    const int row = index.row();
    const int col = index.column();

    if (m_fileType == Core::CanFileType::Raw)
    {
        const auto& buf = std::get<std::vector<Core::RawCanMessage>>(m_buffer);
        if (row >= static_cast<int>(buf.size())) return {};
        const auto& [receiveTime, data, messageId, dlc] = buf[row];

        // Column order mirrors the CN chain in the file
        switch (col)
        {
            case 0:
                return QString::number(static_cast<double>(receiveTime.count()) / 1000.0, 'f', 3);
            case 1:
                return QStringLiteral("1");
            case 2:
                return QString("0x%1").arg(messageId, 3, 16, QChar('0')).toUpper();
            case 3:
                return QStringLiteral("Rx");
            case 4:
                return QString::number(dlc);
            case 5:
                return QString::number(dlc);
            case 6: {
                QString hex;
                for (int i = 0; i < dlc && i < 8; ++i)
                {
                    if (!hex.isEmpty()) hex += ' ';
                    hex += QString("%1")
                               .arg(static_cast<uint8_t>(data[i]), 2, 16, QChar('0'))
                               .toUpper();
                }
                return hex;
            }
            default:
                return {};
        }
    } else
    {
        const auto& buf = std::get<std::vector<Core::DbcCanMessage>>(m_buffer);
        if (row >= static_cast<int>(buf.size())) return {};
        const auto& msg = buf[row];

        if (col == 0)
            return QString::number(static_cast<double>(msg.receiveTime.count()) / 1000.0, 'f', 3);

        const QString colName = m_headers.value(col);
        for (const auto& [name, value] : msg.signalValues)
        {
            if (QString::fromStdString(name) == colName) return QString::number(value, 'f', 3);
        }
        return QStringLiteral("-");
    }
}

auto LoggingDetailProxy::headerData(const int section, const Qt::Orientation orientation,
                                    const int role) const -> QVariant
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    if (section < 0 || section >= m_headers.size()) return {};
    return m_headers.value(section);
}

void LoggingDetailProxy::buildHeaders()
{
    m_headers.clear();
    if (!m_reader) return;

    static const QHash<QString, QString> rawDisplayNames = {
        {"t", "Time (s)"},
        {"CAN_DataFrame.BusChannel", "Channel"},
        {"CAN_DataFrame.ID", "ID"},
        {"CAN_DataFrame.Dir", "Direction"},
        {"CAN_DataFrame.DLC", "DLC"},
        {"CAN_DataFrame.DataLength", "Data Length"},
        {"CAN_DataFrame.DataBytes", "Data"},
    };

    for (const auto& name : m_reader->columnHeaders())
    {
        const QString qname = QString::fromStdString(name);
        if (m_fileType == Core::CanFileType::Raw)
            m_headers.append(rawDisplayNames.value(qname, qname));
        else
            m_headers.append(qname == u"t" ? QStringLiteral("Time (s)") : qname);
    }
}

}  // namespace Logging