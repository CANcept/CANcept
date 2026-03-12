#pragma once

#include <QModelIndex>

#include "core/dto/dbc_dto.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
namespace DbcFile::Util {
/**
 * @brief Returns a sibling index at the specified column (Qt5 compatible).
 * @param idx The original QModelIndex.
 * @param column The target column.
 * @return QModelIndex at the same row and given column.
 */
inline auto siblingAtColumnQt5(const QModelIndex& idx, int column) -> QModelIndex
{
    return idx.sibling(idx.row(), column);
}

/**
 * @brief Checks if the file has a valid DBC extension.
 * @param filePath The file path to check.
 * @return True if the file ends with ".dbc" (case-insensitive), false otherwise.
 */
inline auto isValidFile(const QString& filePath) -> bool
{
    return filePath.endsWith(Constants::LoadPage::FileExt, Qt::CaseInsensitive);
}

/**
 * @brief Checks if a list of files can be dropped in the load area.
 * @param files List of file paths.
 * @return True if exactly one valid DBC file is present.
 */
inline auto canAcceptDrop(const QList<QString>& files) -> bool
{
    return files.size() == 1 && isValidFile(files.first());
}

/**
 * @brief Resolves the numeric message ID from a model index.
 * @param idIdx The QModelIndex containing the ID.
 * @return ID from Role_Id, or fallback to Qt::DisplayRole if 0.
 */
inline auto resolveMessageId(const QModelIndex& idIdx) -> uint
{
    uint id = idIdx.data(DbcFile::DbcRoles::Role_Id).toUInt();
    if (id == 0) id = idIdx.data(Qt::DisplayRole).toUInt();
    return id;
}

/**
 * @brief Extracts all unique signal units from a parsed DBC file to provide filtering options
 * for signals.
 *
 * Iterates over all message definitions and their signals,
 * collects unique unit strings, and returns them sorted
 * alphabetically (case-insensitive).
 *
 * @param config Config to extract units out of.
 * @return A sorted list of unique signal units.
 */
inline auto extractSignalUnits(const Core::DbcConfig& config) -> QStringList
{
    QSet<QString> uniqueUnits;

    for (const auto& msg : config.messageDefinitions)
    {
        for (const auto& sig : msg.signalDescriptions)
        {
            if (!sig.unit.empty())
            {
                uniqueUnits.insert(QString::fromStdString(sig.unit));
            }
        }
    }

    QStringList sortedUnits = uniqueUnits.values();
    sortedUnits.sort(Qt::CaseInsensitive);
    return sortedUnits;
}

/**
 * @brief Extracts all unique message senders from a parsed DBC event to provide filtering
 * options for messages.
 *
 * Iterates over all message definitions and collects unique
 * transmitter names. The result is sorted alphabetically
 * (case-insensitive).
 *
 * @param config Config to extract senders out of.
 * @return A sorted list of unique message sender names.
 */
inline auto extractSenders(const Core::DbcConfig& config) -> QStringList
{
    QSet<QString> uniqueSenders;
    for (const auto& msg : config.messageDefinitions)
    {
        if (!msg.transmitterName.empty())
        {
            uniqueSenders.insert(QString::fromStdString(msg.transmitterName));
        }
    }

    QStringList sortedSenders = uniqueSenders.values();
    sortedSenders.sort(Qt::CaseInsensitive);
    return sortedSenders;
}
}  // namespace DbcFile::Util