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

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "core/interface/i_tab_component.hpp"

/** @brief Default number of restart attempts allowed before fatal exit. */
#ifndef DEFAULT_RESTART_LIMIT
#define DEFAULT_RESTART_LIMIT 2
#endif

namespace AppRoot {

/**
 * @class TabFactory
 * @brief A type-safe factory for dynamically creating and restarting Tab components.
 * * This factory uses C++ Run-Time Type Information (RTTI) to map class types
 * to their respective constructor functions, eliminating the need for manual string IDs.
 */
class TabFactory
{
   public:
    /** @brief Signature for the creation function. */
    using Creator = std::function<std::unique_ptr<Core::ITabComponent>()>;

    TabFactory() = default;
    ~TabFactory() = default;

    /**
     * @brief Registers a constructor function for a specific Tab class.
     * @tparam T The concrete class type to register (must inherit from ITabComponent).
     * @param creator A lambda or function returning a unique_ptr to the new instance.
     */
    template <typename T>
    void registerCreator(Creator creator)
    {
        m_creators[std::type_index(typeid(T))] = {std::move(creator), DEFAULT_RESTART_LIMIT};
    }

    /**
     * @brief Creates a new instance of a tab based on its compile-time type.
     * @tparam T The class type to instantiate.
     * @return std::unique_ptr<Core::ITabComponent> The new instance, or nullptr if not registered.
     */
    template <typename T>
    auto create() const -> std::unique_ptr<Core::ITabComponent>
    {
        return createByTypeIndex(std::type_index(typeid(T)));
    }

    /**
     * @brief Creates a new instance based on a runtime type index.
     * @param index The std::type_index of the class.
     * @return std::unique_ptr<Core::ITabComponent> The new instance, or nullptr if not registered.
     */
    auto createByTypeIndex(std::type_index index) const -> std::unique_ptr<Core::ITabComponent>;

    /**
     * @brief Checks if a specific type is registered in the factory.
     * @param index The std::type_index to check.
     * @return true If a creator exists for this type.
     */
    auto isRegistered(std::type_index index) const -> bool;

    /**
     * @brief Validates if a module is allowed to restart and decrements its internal counter.
     * @param index The std::type_index of the class.
     * @return true If restarts are still available, false if the limit is reached.
     */
    auto canRestart(std::type_index index) -> bool;

   private:
    /** @brief Internal storage for creator and restart state. */
    struct TabEntry {
        Creator creator;
        int remainingRestarts;
    };

    /** @brief Map of type identities to their respective entries. */
    std::unordered_map<std::type_index, TabEntry> m_creators;
};

}  // namespace AppRoot