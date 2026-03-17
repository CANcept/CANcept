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

#include "tab_factory.hpp"

#include "core/macro/console_logging.hpp"

namespace AppRoot {

std::unique_ptr<Core::ITabComponent> TabFactory::createByTypeIndex(
    const std::type_index index) const
{
    if (const auto it = m_creators.find(index); it != m_creators.end())
    {
        return it->second.creator();
    }

    // Log error if type is unknown (using type name for debugging)
    LOG_ERR("TabFactory", "TabFactory: No creator registered for type: {}", index.name());
    return nullptr;
}

bool TabFactory::isRegistered(const std::type_index index) const
{
    return m_creators.find(index) != m_creators.end();
}

auto TabFactory::canRestart(const std::type_index index) -> bool
{
    if (const auto it = m_creators.find(index); it != m_creators.end())
    {
        if (it->second.remainingRestarts > 0)
        {
            it->second.remainingRestarts--;
            return true;
        }
    }
    return false;
}

}  // namespace AppRoot
