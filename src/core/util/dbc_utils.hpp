
#pragma once
#include <QModelIndex>
#include <QString>

namespace Core::Util {

/**
 * @brief Formats a CAN ID to hex value
 *  * Example: 255 -> "0x0FF"
 */
inline QString formatId(uint id)
{
    return QStringLiteral("0x%1").arg(id, 3, 16, QChar('0')).toUpper();
}

/**
 * @brief Formats numeric values to a more compact form.
 * Example: 40.000 -> "40", 0.125 -> "0.125"
 */
inline QString formatNumber(double value)
{
    return QString::number(value, 'g', 12);
}

/**
 * @brief Formats min and max values to a range string.
 * Example: min = 0, max = 100 -> [0, 100]
 */
inline QString formatRange(const uint min, const uint max)
{
    return QString("[%1, %2]").arg(min).arg(max);
}

inline auto siblingAtColumnQt5(const QModelIndex& idx, int column) -> QModelIndex
{
    return idx.sibling(idx.row(), column);
}

}  // namespace Core::Util