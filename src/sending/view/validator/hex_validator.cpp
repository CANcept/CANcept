#include "hex_validator.hpp"

#include "sending/constants.hpp"

namespace Sending {

HexValidator::HexValidator(const std::optional<uint64_t> minValue,
                           const std::optional<uint64_t> maxValue, QObject* parent)
    : QValidator(parent), m_minValue(minValue), m_maxValue(maxValue)
{
}

QValidator::State HexValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);
    if (input.isEmpty()) return Intermediate;

    // Hex Check
    if (!Constants::HEX_VALIDATION_PATTERN.match(input).hasMatch()) return Invalid;
    input = input.toUpper();

    // Range Check
    bool ok;
    const uint64_t value = input.toULongLong(&ok, 16);
    if (!ok) return Invalid;

    if (m_maxValue.has_value())
    {
        if (value > m_maxValue.value()) return Invalid;
    }
    if (m_minValue.has_value() && value < m_minValue.value())
    {
        return Intermediate;
    }

    return Acceptable;
}

void HexValidator::setMaxValue(uint64_t maxValue)
{
    m_maxValue = maxValue;
}

void HexValidator::setMinValue(uint64_t minValue)
{
    m_minValue = minValue;
}

}  // namespace Sending
