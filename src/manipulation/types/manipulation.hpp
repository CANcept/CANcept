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
#include <variant>
#include <vector>

#include "effect/bit_flip_effect.hpp"
#include "effect/clamp_effect.hpp"
#include "effect/noise_injection_effect.hpp"
#include "effect/random_bit_flip_effect.hpp"
#include "effect/value_set_effect.hpp"
#include "mutation/latch_mutation.hpp"
#include "mutation/no_mutation.hpp"
#include "strategy/delayed_strategy.hpp"
#include "strategy/drop_strategy.hpp"
#include "strategy/effect_strategy.hpp"
#include "strategy/insert_strategy.hpp"
#include "trigger/dlc_trigger.hpp"
#include "trigger/id_trigger.hpp"
#include "trigger/random_trigger.hpp"
#include "trigger/signal_name_trigger.hpp"
#include "trigger/signal_threshold_trigger.hpp"

namespace Manipulation {

/**
 * To avoid vtable lookup and dislocated locality because of polymorphism, variants are used.
 */
using DbcTrigger = std::variant<SignalNameTrigger, SignalThresholdTrigger, RandomTrigger>;
using RawTrigger = std::variant<IDTrigger, DLCTrigger, RandomTrigger>;

using DbcEffect = std::variant<ValueSetEffect, ClampEffect, NoiseEffect>;
using RawEffect = std::variant<BitFlipEffect, RandomBitFlipEffect>;

using RawEffectStrategy = EffectStrategy<RawEffect>;
using DbcEffectStrategy = EffectStrategy<DbcEffect>;

using RawStrategy = std::variant<RawEffectStrategy, DelayedStrategy, DropStrategy>;
using DbcStrategy =
    std::variant<DbcEffectStrategy, DelayedStrategy, DropStrategy, DbcInsertStrategy>;

using Mutation = std::variant<NoMutation, LatchMutation>;

/**
 * @struct RawManipulation
 * @brief This is the blueprint for a generic raw manipulation.
 */
struct RawManipulation {
    std::vector<RawTrigger> trigger;
    RawStrategy strategy;
    Mutation mutation;
};

/**
 * @struct DbcManipulation
 * @brief This is the blueprint for a generic dbc manipulation.
 */
struct DbcManipulation {
    std::vector<DbcTrigger> trigger;
    DbcStrategy strategy;
    Mutation mutation;
};

using ManipulationEntry = std::variant<RawManipulation, DbcManipulation>;

}  // namespace Manipulation