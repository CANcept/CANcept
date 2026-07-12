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

#include "manipulation_serializer.hpp"

#include "core/service/serializer.hpp"

namespace Manipulation {

auto ManipulationSerializer::save(const std::filesystem::path& path,
                                  const std::vector<ManipulationEntry>& entries) -> bool
{
    auto snapshot = entries;
    Core::Serializer serializer;
    serializer.addItem("manipulations", snapshot);
    return serializer.saveToFile(path);
}

auto ManipulationSerializer::load(const std::filesystem::path& path)
    -> std::vector<ManipulationEntry>
{
    std::vector<ManipulationEntry> entries;
    Core::Serializer serializer;
    serializer.addItem("manipulations", entries);
    serializer.loadFromFile(path);
    return entries;
}

}  // namespace Manipulation
