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
