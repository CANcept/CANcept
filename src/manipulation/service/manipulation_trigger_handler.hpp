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
#include <algorithm>
#include <random>
#include <string>
#include <unordered_map>

#include "core/dto/can_dto.hpp"
#include "entt/core/utility.hpp"
#include "manipulation/types/manipulation.hpp"

namespace Manipulation {

/**
 * Checks weather the trigger fires for this message.
 *
 * @param trigger the trigger for the manipulation
 * @param id the identifier of the message
 * @param dlc the dlc
 * @param data the data of the message
 * @param random a random instance
 * @return if the trigger was fired
 */
inline auto firesRawTrigger(const RawTrigger& trigger, const uint16_t& id, const uint8_t& dlc,
                            const std::array<char, 8>& data, std::mt19937& random) -> bool
{
    (void)data;
    return std::visit(entt::overloaded{
                          // lambda to check if the message has a certain id
                          [&](const IDTrigger& t) { return id == t.id; },

                          // lambda to check if the message has a certain dlc
                          [&](const DLCTrigger& t) { return dlc == t.dlc; },

                          // lambda triggering randomly
                          [&](const RandomTrigger& t) {
                              return std::generate_canonical<float, 10>(random) <= t.probability;
                          },
                      },
                      trigger);
}

/**
 * Checks weather the trigger fires for this message.
 *
 * @param trigger the trigger for the manipulation
 * @param signalMap the signals
 * @param random a random instance
 * @return if the trigger was fired
 */
inline auto firesDbcTrigger(const DbcTrigger& trigger,
                            const std::unordered_map<std::string, Core::DbcCanSignal*>& signalMap,
                            std::mt19937& random) -> bool
{
    return std::visit(
        entt::overloaded{
            [&](const SignalNameTrigger& t) -> bool { return signalMap.contains(t.signal_name); },
            [&](const SignalThresholdTrigger& t) -> bool {
                const auto it = signalMap.find(t.signal_name);
                if (it == signalMap.end())
                {
                    return false;
                }
                return t.isGreater ? it->second->value >= t.threshold
                                   : it->second->value <= t.threshold;
            },
            [&](const RandomTrigger& t) -> bool {
                return std::generate_canonical<float, 10>(random) <= t.probability;
            }},
        trigger);
}

}  // namespace Manipulation