#pragma once
#include <cstring>
#include <random>

#include "core/dto/can_dto.hpp"
#include "entt/core/utility.hpp"
#include "fault_injector/types/Fault.hpp"
#include "fault_injector/types/effect/bit_flip_effect.hpp"
#include "fault_injector/types/effect/random_bit_flip_effect.hpp"

namespace FaultInjector {

/**
 *  This function applies raw effects to the given message. It is not Thread safe.
 *
 * @param effect the effect to apply
 * @param message the message the effect should be applied to
 * @param random the curent random instance for probability
 */
inline void applyRawEffect(const RawEffect& effect, Core::RawCanMessage& message,
                           std::mt19937& random)
{
    std::visit(entt::overloaded{
                   // lambda for flipping a pre determined bit
                   [&](const BitFlipEffect& e) { message.data[e.byteIndex] ^= (1 << e.bitIndex); },

                   // lambda for flipping a random bit
                   [&](const RandomBitFlipEffect&) {
                       static std::uniform_int_distribution<size_t> byteDist(0, 7);
                       static std::uniform_int_distribution<uint8_t> bitDist(0, 7);
                       message.data[byteDist(random)] ^= (1U << bitDist(random));
                   }},
               effect);
}

/**
 *  This function applies dbc based effects to the given message. It is not Thread safe.
 *
 * @param effect the effect to apply
 * @param message the message the effect should be applied to
 * @param random the curent random instance for probability
 */
inline void applyDbcEffect(const DbcEffect& effect, Core::DbcCanMessage& message,
                           const std::mt19937& random)
{
    std::visit(
        entt::overloaded{
            // lambda for setting the value of a certain signal
            [&](const ValueSetEffect& e) {
                const auto it = std::ranges::find_if(message.signalValues, [&](const auto& signal) {
                    return signal.name == e.signal_name;
                });
                if (it != message.signalValues.end())
                {
                    it->value = e.value;
                }
            },
        },
        effect);
    (void)random;
}

}  // namespace FaultInjector