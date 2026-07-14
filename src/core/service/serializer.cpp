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

#include "serializer.hpp"

#include <fstream>

#include "core/macro/console_logging.hpp"

namespace Core {

void Serializer::addItem(std::string key, std::function<nlohmann::json()> encode,
                         std::function<void(const nlohmann::json&)> decode)
{
    m_items.push_back(
        Item{.key = std::move(key), .encode = std::move(encode), .decode = std::move(decode)});
}

auto Serializer::toJson() const -> nlohmann::json
{
    auto json = nlohmann::json::object();
    for (const auto& item : m_items)
    {
        json[item.key] = item.encode();
    }
    return json;
}

void Serializer::fromJson(const nlohmann::json& json)
{
    for (const auto& item : m_items)
    {
        if (json.contains(item.key))
        {
            item.decode(json.at(item.key));
        }
    }
}

auto Serializer::saveToFile(const std::filesystem::path& path) const -> bool
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        LOG_ERR("Serializer", "Failed to open file for writing: {}", path.string());
        return false;
    }

    stream << toJson().dump(4);
    return true;
}

auto Serializer::loadFromFile(const std::filesystem::path& path) -> bool
{
    std::ifstream stream(path);
    if (!stream.is_open())
    {
        LOG_ERR("Serializer", "Failed to open file for reading: {}", path.string());
        return false;
    }

    nlohmann::json json;
    try
    {
        stream >> json;
    } catch (const nlohmann::json::parse_error& e)
    {
        LOG_ERR("Serializer", "Failed to parse JSON from file {}: {}", path.string(), e.what());
        return false;
    }

    fromJson(json);
    return true;
}

}  // namespace Core
