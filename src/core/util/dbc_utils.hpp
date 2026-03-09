#pragma once
#include <QString>

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
}  // namespace DbcFile::Util