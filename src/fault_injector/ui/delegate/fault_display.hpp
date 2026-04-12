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
#include <QString>

#include "entt/core/utility.hpp"
#include "fault_injector/types/fault.hpp"

namespace FaultInjector {

/**
 * @brief Returns the display name of a given raw trigger.
 *
 * @param trigger the trigger for the name
 * @return the display name
 */
inline QString triggerLabel(const RawTrigger& trigger)
{
    return std::visit(
        entt::overloaded{
            [](const IDTrigger& t) { return QStringLiteral("ID: 0x%1").arg(t.id, 0, 16); },
            [](const DLCTrigger& t) { return QStringLiteral("DLC: %1").arg(t.dlc); },
            [](const RandomTrigger& t) {
                return QStringLiteral("Random (%1%)").arg(t.probability * 100);
            },
        },
        trigger);
}

/**
 * @brief Returns the display name of a given dbc trigger.
 *
 * @param trigger the trigger for the name
 * @return the display name
 */
inline QString triggerLabel(const DbcTrigger& trigger)
{
    return std::visit(entt::overloaded{
                          [](const SignalNameTrigger& t) {
                              return QStringLiteral("Signal: %1").arg(t.signal_name.data());
                          },
                          [](const RandomTrigger& t) {
                              return QStringLiteral("Random (%1%)").arg(t.probability * 100);
                          },
                      },
                      trigger);
}

/**
 * @brief Returns the display name of a given raw effect.
 *
 * @param effect the effect for the name
 * @return the display name
 */
inline QString effectLabel(const RawEffect& effect)
{
    return std::visit(
        entt::overloaded{
            [](const BitFlipEffect& e) {
                return QStringLiteral("Bit flip %1 @ %2").arg(e.byteIndex).arg(e.bitIndex);
            },
            [](const RandomBitFlipEffect&) { return QStringLiteral("Random bit flip"); },
        },
        effect);
}

/**
 * @brief Returns the display name of a given dbc effect.
 *
 * @param effect the effect for the name
 * @return the display name
 */
inline QString effectLabel(const DbcEffect& effect)
{
    return std::visit(
        entt::overloaded{
            [](const ValueSetEffect& e) { return QStringLiteral("Set value: %1").arg(e.value); },
            [](const ClampEffect& e) {
                return QStringLiteral("Clamp [%1, %2]").arg(e.min_value).arg(e.max_value);
            },
            [](const NoiseEffect& e) { return QStringLiteral("Noise ±%1").arg(e.amplitude); },
        },
        effect);
}

/**
 * @brief Returns the display name of a given mutation.
 *
 * @param mutation the mutation for the name
 * @return the display name
 */
inline QString mutationLabel(const Mutation& mutation)
{
    return std::visit(entt::overloaded{
                          [](const NoMutation&) { return QStringLiteral("None"); },
                          [](const LatchMutation&) { return QStringLiteral("Latch"); },
                      },
                      mutation);
}

/**
 * @brief Returns the display name of a given strategy.
 *
 * @param strategy the strategy for the name
 * @return the display name
 */
inline QString strategyLabel(const Strategy& strategy)
{
    return std::visit(
        entt::overloaded{
            [](const ImmediateStrategy&) { return QStringLiteral("Immediate"); },
            [](const DelayedStrategy& s) { return QStringLiteral("Delayed %1µs").arg(s.delayUs); },
            [](const DropStrategy&) { return QStringLiteral("Drop"); },
        },
        strategy);
}

}  // namespace FaultInjector