#pragma once
#include <variant>
#include <vector>

#include "effect/bit_flip_effect.hpp"
#include "effect/random_bit_flip_effect.hpp"
#include "effect/value_set_effect.hpp"
#include "strategy/immediate_strategy.hpp"
#include "trigger/dlc_trigger.hpp"
#include "trigger/id_trigger.hpp"
#include "trigger/random_trigger.hpp"
#include "trigger/signal_name_trigger.hpp"

namespace FaultInjector {

/**
 * To avoid vtable lookup and dislocated locality because of polymorphism, variants are used.
 */
using DbcTrigger = std::variant<SignalNameTrigger, RandomTrigger>;
using RawTrigger = std::variant<IDTrigger, DLCTrigger, RandomTrigger>;

using DbcEffect = std::variant<ValueSetEffect>;
using RawEffect = std::variant<BitFlipEffect, RandomBitFlipEffect>;

using Strategy = std::variant<ImmediateStrategy>;

/**
 * @struct RawFault
 * @brief This is the blueprint for a generic raw fault.
 */
struct RawFault {
    std::vector<RawTrigger> trigger;
    std::vector<RawEffect> effect;
    Strategy strategy;
};

/**
 * @struct DbcFault
 * @brief This is the blueprint for a generic dbc fault.
 */
struct DbcFault {
    std::vector<DbcTrigger> trigger;
    std::vector<DbcEffect> effect;
    Strategy strategy;
};

using Fault = std::variant<RawFault, DbcFault>;

}  // namespace FaultInjector