#pragma once
#include <algorithm>
#include <random>

#include "core/dto/can_dto.hpp"
#include "entt/core/utility.hpp"
#include "fault_injector/types/Fault.hpp"

namespace FaultInjector {

/**
 * Checks wether the trigger fires for this message.
 *
 * @param trigger the trigger for the fault
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
 * Checks wether the trigger fires for this message.
 *
 * @param trigger the trigger for the fault
 * @param message the message which may be triggering a fault
 * @param random a random instance
 * @return if the trigger was fired
 */
inline auto firesDbcTrigger(const DbcTrigger& trigger, const Core::DbcCanMessage& message,
                            std::mt19937& random) -> bool
{
    return std::visit(
        entt::overloaded{// lambda triggering when the message has a signal with the given name
                         [&](const SignalNameTrigger& t) {
                             return std::ranges::any_of(
                                 message.signalValues,
                                 [&](const auto& signal) { return signal.name == t.signal_name; });
                         },

                         // lambda triggering randomly
                         [&](const RandomTrigger& t) {
                             return std::generate_canonical<float, 10>(random) <= t.probability;
                         }},
        trigger);
}

}  // namespace FaultInjector