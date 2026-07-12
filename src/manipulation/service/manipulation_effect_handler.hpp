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
#include <cstring>
#include <random>
#include <string>
#include <unordered_map>

#include "core/dto/can_dto.hpp"
#include "entt/core/utility.hpp"
#include "manipulation/types/effect/bit_flip_effect.hpp"
#include "manipulation/types/effect/random_bit_flip_effect.hpp"
#include "manipulation/types/manipulation.hpp"

namespace Manipulation {

/**
 *  This function applies raw effects to the given message. It is not Thread safe.
 *
 * @param effect the effect to apply
 * @param id the identifier of the message
 * @param dlc the dlc
 * @param data the data of the message
 * @param random the curent random instance for probability
 */
inline void applyRawEffect(const RawEffect& effect, const uint16_t& id, const uint8_t& dlc,
                           std::array<char, 8>& data, std::mt19937& random)
{
    (void)id;
    (void)dlc;
    std::visit(
        entt::overloaded{// lambda for flipping a pre determined bit
                         [&](const BitFlipEffect& e) { data[e.byteIndex] ^= (1 << e.bitIndex); },

                         // lambda for flipping a random bit
                         [&](const RandomBitFlipEffect&) {
                             static std::uniform_int_distribution<size_t> byteDist(0, 7);
                             static std::uniform_int_distribution<uint8_t> bitDist(0, 7);
                             data[byteDist(random)] ^= (1U << bitDist(random));
                         }},
        effect);
}

/**
 *  This function applies dbc based effects to the given message. It is not Thread safe.
 *
 * @param effect the effect to apply
 * @param signalMap the signals
 * @param random the curent random instance for probability
 */
inline void applyDbcEffect(const DbcEffect& effect,
                           std::unordered_map<std::string, Core::DbcCanSignal*>& signalMap,
                           std::mt19937& random)
{
    std::visit(entt::overloaded{
                   [&](const ValueSetEffect& e) {
                       if (const auto it = signalMap.find(e.signal_name); it != signalMap.end())
                       {
                           it->second->value = e.value;
                       }
                   },
                   [&](const ClampEffect& e) {
                       if (const auto it = signalMap.find(e.signal_name); it != signalMap.end())
                       {
                           it->second->value =
                               std::clamp(it->second->value, e.min_value, e.max_value);
                       }
                   },
                   [&](const NoiseEffect& e) {
                       std::uniform_real_distribution<double> dist(-e.amplitude, e.amplitude);
                       if (const auto it = signalMap.find(e.signal_name); it != signalMap.end())
                       {
                           it->second->value += dist(random);
                       }
                   },
               },
               effect);
}

}  // namespace Manipulation