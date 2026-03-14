#pragma once
#include <QString>

namespace Core {

/**
 * @brief Formats a CAN ID as a hexadecimal string.
 * @param id The CAN ID to format.
 * @return Hex string with "0x" prefix and 3 digits, e.g., 255 -> "0x0FF".
 */
inline auto formatId(uint id) -> QString
{
    return QStringLiteral("0x") + QString("%1").arg(id, 3, 16, QChar('0')).toUpper();
}

/**
 * @brief Formats a numeric value into a compact string representation.
 * @param value The numeric value to format.
 * @return A string with no unnecessary trailing zeros, e.g., 40.0 -> "40", 0.125 -> "0.125".
 */
inline auto formatNumber(double value) -> QString
{
    return QString::number(value, 'g', 12);
}

/**
 * @brief Formats a numeric range with potential fractional values as a string.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return A string in the format "[min, max]", preserving fractional parts, e.g., 0.1 and 100.5 ->
 * "[0.1, 100.5]".
 */
inline auto formatRange(const double min, const double max) -> QString
{
    return QString("[%1, %2]").arg(formatNumber(min), formatNumber(max));
}
}  // namespace Core