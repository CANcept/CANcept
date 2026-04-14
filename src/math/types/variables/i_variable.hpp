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
#include <string>

namespace Math {

/**
 * @brief Contract for a variable whose value is updated each cycle before evaluation.
 *
 * Variables are identified by a config key for deduplication in the registry,
 * and provide a display name for UI purposes. Symbols are assigned per-model,
 * not per-variable.
 */
class IVariable
{
   public:
    virtual ~IVariable() = default;

    /**
     * @brief Unique identity key used for deduplication in the registry.
     *
     * Examples: "time:seconds", "signal:256:EngineRPM"
     */
    virtual auto configKey() const -> std::string = 0;

    /**
     * @brief Human-readable label shown in the UI.
     *
     * Examples: "Time (s)", "MotorMsg.EngineRPM"
     */
    virtual auto displayName() const -> std::string = 0;

    /**
     * @brief Resets the variable to the value from the start.
     */
    virtual void reset() = 0;

    /**
     * @brief Returns a raw pointer to the underlying value for exprtk registration.
     */
    virtual auto ptr() -> double* = 0;

    /**
     * @brief Returns a shared pointer to the value, keeping it alive across registry changes.
     */
    virtual auto sharedPtr() -> std::shared_ptr<double> = 0;

    /**
     * @brief Refreshes the variable's value for the current evaluation cycle.
     */
    virtual void update() = 0;
};

}  // namespace Math
