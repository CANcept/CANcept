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

#pragma once
#include <optional>

#include "manipulation/constants.hpp"
#include "manipulation/types/manipulation.hpp"
#include "section_entry.hpp"

namespace Manipulation {

/**
 * @brief Constructs a RawTrigger based on the entry.
 * @param entry the trigger to construct the trigger from
 * @return the effect
 */
inline auto buildRawTrigger(const SectionEntry& entry) -> std::optional<RawTrigger>
{
    switch (entry.typeIndex)
    {
        case 0: {
            bool ok = false;
            const uint id =
                entry.params.value(Constants::PARAM_ID_INPUT).toString().toUInt(&ok, 16);
            if (!ok || id > 0x7FFu)
            {
                return IDTrigger(0);
            }
            return IDTrigger(static_cast<uint16_t>(id));
        }
        case 1:
            return DLCTrigger(
                static_cast<uint8_t>(entry.params.value(Constants::PARAM_DLC_INPUT, 0).toInt()));
        case 2:
            return RandomTrigger(static_cast<float>(
                entry.params.value(Constants::PARAM_PROB_INPUT, 0.5).toDouble()));
        default:
            return std::nullopt;
    }
}

/**
 * @brief Constructs a DbcTrigger based on the entry.
 * @param entry the trigger to construct the trigger from
 * @return the effect
 */
inline auto buildDbcTrigger(const SectionEntry& entry) -> std::optional<DbcTrigger>
{
    switch (entry.typeIndex)
    {
        case 0: {
            const QString name =
                entry.params.value(Constants::PARAM_SIGNAL_NAME_INPUT).toString().trimmed();
            if (name.isEmpty())
            {
                return std::nullopt;
            }
            return SignalNameTrigger(name.toStdString());
        }
        case 1: {
            const QString name =
                entry.params.value(Constants::PARAM_SIGNAL_NAME_INPUT).toString().trimmed();
            if (name.isEmpty())
            {
                return std::nullopt;
            }
            const bool isGreater =
                entry.params.value(Constants::PARAM_IS_GREATER_INPUT, 0).toInt() == 0;
            const double threshold =
                entry.params.value(Constants::PARAM_THRESHOLD_INPUT, 0.0).toDouble();
            return SignalThresholdTrigger(name.toStdString(), threshold, isGreater);
        }
        case 2:
            return RandomTrigger(static_cast<float>(
                entry.params.value(Constants::PARAM_PROB_INPUT, 0.5).toDouble()));
        default:
            return std::nullopt;
    }
}

/**
 * @brief Constructs a RawEffect based on the entry.
 * @param entry the entry to construct the effect from
 * @return the effect
 */
inline auto buildRawEffect(const SectionEntry& entry) -> std::optional<RawEffect>
{
    switch (entry.typeIndex)
    {
        case 0:
            return BitFlipEffect(
                static_cast<uint8_t>(entry.params.value(Constants::PARAM_BYTE_INPUT, 0).toInt()),
                static_cast<uint8_t>(entry.params.value(Constants::PARAM_BIT_INPUT, 0).toInt()));
        case 1:
            return RandomBitFlipEffect{};
        default:
            return std::nullopt;
    }
}

/**
 * @brief Constructs a DbcEffect based on the entry.
 * @param entry the entry to construct the effect from
 * @return the effect
 */
inline auto buildDbcEffect(const SectionEntry& entry) -> std::optional<DbcEffect>
{
    switch (entry.typeIndex)
    {
        case 0: {
            const QString name =
                entry.params.value(Constants::PARAM_SIGNAL_NAME_INPUT).toString().trimmed();
            if (name.isEmpty())
            {
                return std::nullopt;
            }
            return ValueSetEffect(name.toStdString(),
                                  entry.params.value(Constants::PARAM_VALUE_INPUT, 0.0).toDouble());
        }
        case 1: {
            const QString name =
                entry.params.value(Constants::PARAM_SIGNAL_NAME_INPUT).toString().trimmed();
            if (name.isEmpty())
            {
                return std::nullopt;
            }
            return ClampEffect(
                name.toStdString(),
                entry.params.value(Constants::PARAM_MIN_VALUE_INPUT, 0.0).toDouble(),
                entry.params.value(Constants::PARAM_MAX_VALUE_INPUT, 100.0).toDouble());
        }
        case 2: {
            const QString name =
                entry.params.value(Constants::PARAM_SIGNAL_NAME_INPUT).toString().trimmed();
            if (name.isEmpty())
            {
                return std::nullopt;
            }
            return NoiseEffect(
                name.toStdString(),
                entry.params.value(Constants::PARAM_AMPLITUDE_INPUT, 1.0).toDouble());
        }
        default:
            return std::nullopt;
    }
}

/**
 * @brief Constructs a Mutation based on the entry.
 * @return the mutation
 */
inline auto buildMutation(const SectionEntry& entry) -> Mutation
{
    switch (entry.typeIndex)
    {
        case 1:
            return LatchMutation{};
        default:
            return NoMutation{};
    }
}

/**
 * @brief Constructs a Strategy based on the entry.
 * @return the strategy
 */
inline auto buildStrategy(const SectionEntry& entry) -> Strategy
{
    switch (entry.typeIndex)
    {
        case 1:
            return DelayedStrategy(static_cast<uint16_t>(
                entry.params.value(Constants::PARAM_DELAY_INPUT, 1000).toInt()));
        case 2:
            return DropStrategy{};
        default:
            return ImmediateStrategy{};
    }
}

}  // namespace Manipulation