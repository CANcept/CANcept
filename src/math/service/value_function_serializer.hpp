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

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "math/service/value_function.hpp"
#include "math/types/variables/i_variable.hpp"
#include "math/ui/model/math_input_model.hpp"

namespace Core {
struct DbcConfig;
}

namespace Math {

class VariableRegistry;

/**
 * @brief Builds an IVariable purely from its configKey (e.g. "signal:256:EngineRPM",
 * "time:seconds"), independent of the QWidget-based IVariableTypeProvider factories used for
 * interactive creation in the UI.
 *
 * @param dbcConfig Used to resolve a signal variable's cosmetic message name; may be null.
 * @return nullptr if configKey doesn't match a known prefix.
 */
auto makeVariableFromConfigKey(const std::string& configKey,
                               const Core::DbcConfig* dbcConfig) -> std::unique_ptr<IVariable>;

/**
 * @brief Saves/loads a MathInputModel's expression tree and variable bindings to/from disk.
 */
class ValueFunctionSerializer
{
   public:
    [[nodiscard]] static auto encode(const MathInputModel& model) -> nlohmann::json;

    /**
     * @brief Rebuilds the expression tree and variable bindings into model, re-acquiring each
     * variable from registry, and returns a parsed ValueFunction over the new tree.
     */
    [[nodiscard]] static auto decode(const nlohmann::json& json, MathInputModel& model,
                                     VariableRegistry& registry) -> std::unique_ptr<ValueFunction>;

    [[nodiscard]] static auto save(const std::filesystem::path& path,
                                   const MathInputModel& model) -> bool;
    [[nodiscard]] static auto load(const std::filesystem::path& path, MathInputModel& model,
                                   VariableRegistry& registry) -> std::unique_ptr<ValueFunction>;
};

}  // namespace Math
