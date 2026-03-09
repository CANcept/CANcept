#pragma once

#include <QModelIndex>

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
class QModelIndex;
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
inline bool isValidFile(const QString& filePath)
{
    return filePath.endsWith(DbcFile::Constants::LoadPage::FileExt, Qt::CaseInsensitive);
}

/**
 * @brief Checks if a list of files can be dropped in the load area.
 * @param files List of file paths.
 * @return True if exactly one valid DBC file is present.
 */
inline bool canAcceptDrop(const QList<QString>& files)
{
    return files.size() == 1 && isValidFile(files.first());
}

/**
 * @brief Resolves the numeric message ID from a model index.
 * @param idIdx The QModelIndex containing the ID.
 * @return ID from Role_Id, or fallback to Qt::DisplayRole if 0.
 */
inline uint resolveMessageId(const QModelIndex& idIdx)
{
    uint id = idIdx.data(DbcFile::DbcRoles::Role_Id).toUInt();
    if (id == 0) id = idIdx.data(Qt::DisplayRole).toUInt();
    return id;
}
}  // namespace DbcFile::Util