#pragma once
#include <QList>
#include <QModelIndex>
#include <QString>
#include <dbc_file/constants.hpp>
#include <dbc_file/model/dbc_roles.hpp>

namespace DbcFile::Util {

/**
 * @brief Formats a CAN ID as a hexadecimal string.
 * @param id The CAN ID to format.
 * @return Hex string with "0x" prefix and 3 digits, e.g., 255 -> "0x0FF".
 */
inline QString formatId(uint id)
{
    return QStringLiteral("0x") + QString("%1").arg(id, 3, 16, QChar('0')).toUpper();
}

/**
 * @brief Formats a numeric value into a compact string representation.
 * @param value The numeric value to format.
 * @return A string with no unnecessary trailing zeros, e.g., 40.0 -> "40", 0.125 -> "0.125".
 */
inline QString formatNumber(double value)
{
    return QString::number(value, 'g', 12);
}

/**
 * @brief Formats a numeric range as a string.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return A string in the format "[min, max]", e.g., 0 and 100 -> "[0, 100]".
 */
inline QString formatRange(const uint min, const uint max)
{
    return QString("[%1, %2]").arg(min).arg(max);
}

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