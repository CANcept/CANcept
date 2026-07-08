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
#include <memory>
#include <vector>

#include "core/dto/dbc_dto.hpp"
#include "i_manipulation_row_type_provider.hpp"

namespace Manipulation {

/** @brief Providers for Raw triggers */
auto getRawTriggerProviders() -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for DBC triggers */
auto getDbcTriggerProviders(const Core::DbcConfig* dbcConfig)
    -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for Raw effects */
auto getRawEffectProviders() -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for DBC effects */
auto getDbcEffectProviders(const Core::DbcConfig* dbcConfig)
    -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for Raw strategies */
auto getRawStrategyProviders() -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for DBC strategies */
auto getDbcStrategyProviders() -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

/** @brief Providers for mutations */
auto getMutationProviders() -> std::vector<std::unique_ptr<IManipulationRowTypeProvider>>;

}  // namespace Manipulation