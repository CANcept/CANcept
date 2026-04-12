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

#include "fault_row_type_providers.hpp"

#include "effect/bit_flip_effect_provider.hpp"
#include "effect/clamp_effect_provider.hpp"
#include "effect/noise_effect_provider.hpp"
#include "effect/random_bit_flip_effect_provider.hpp"
#include "effect/value_set_effect_provider.hpp"
#include "mutation/latch_mutation_provider.hpp"
#include "mutation/no_mutation_provider.hpp"
#include "strategy/delayed_strategy_provider.hpp"
#include "strategy/drop_strategy_provider.hpp"
#include "strategy/immediate_strategy_provider.hpp"
#include "trigger/dlc_trigger_provider.hpp"
#include "trigger/id_trigger_provider.hpp"
#include "trigger/random_trigger_provider.hpp"
#include "trigger/signal_name_trigger_provider.hpp"

namespace FaultInjector {

auto getRawTriggerProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<IdTriggerProvider>());
    vector.push_back(std::make_unique<DlcTriggerProvider>());
    vector.push_back(std::make_unique<RandomTriggerProvider>());
    return vector;
}

auto getDbcTriggerProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<SignalNameTriggerProvider>());
    vector.push_back(std::make_unique<RandomTriggerProvider>());
    return vector;
}

auto getRawEffectProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<BitFlipEffectProvider>());
    vector.push_back(std::make_unique<RandomBitFlipEffectProvider>());
    return vector;
}

auto getDbcEffectProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<ValueSetEffectProvider>());
    vector.push_back(std::make_unique<ClampEffectProvider>());
    vector.push_back(std::make_unique<NoiseEffectProvider>());
    return vector;
}

auto getStrategyProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<ImmediateStrategyProvider>());
    vector.push_back(std::make_unique<DelayedStrategyProvider>());
    vector.push_back(std::make_unique<DropStrategyProvider>());
    return vector;
}

auto getMutationProviders() -> std::vector<std::unique_ptr<IFaultRowTypeProvider>>
{
    std::vector<std::unique_ptr<IFaultRowTypeProvider>> vector;
    vector.push_back(std::make_unique<NoMutationProvider>());
    vector.push_back(std::make_unique<LatchMutationProvider>());
    return vector;
}

}  // namespace FaultInjector