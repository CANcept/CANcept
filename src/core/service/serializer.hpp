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
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace Core {

/**
 * @brief Builds a JSON document out of named items and reads it back into the same items.
 *
 * A caller registers each piece of state it wants persisted via addItem(), then calls
 * toJson()/saveToFile() to encode it, or fromJson()/loadFromFile() to decode it. Because the
 * same registration is used for both directions, decoding writes straight back into the
 * original bound reference (or does whatever a custom decode closure captured), so every item
 * keeps its concrete type instead of round-tripping through a generic JSON value.
 */
class Serializer
{
   public:
    /**
     * @brief Registers a reference whose type has ADL-visible to_json/from_json overloads.
     */
    template <typename T>
    void addItem(std::string key, T& value)
    {
        addItem(
            std::move(key), [&value] { return nlohmann::json(value); },
            [&value](const nlohmann::json& json) { value = json.get<T>(); });
    }

    /**
     * @brief Registers an item via explicit encode/decode closures.
     *
     * Use this when decoding needs context beyond the JSON itself (e.g. a registry lookup).
     */
    void addItem(std::string key, std::function<nlohmann::json()> encode,
                 std::function<void(const nlohmann::json&)> decode);

    /**
     * @brief Encodes every registered item into a single JSON object, keyed by its name.
     */
    [[nodiscard]] auto toJson() const -> nlohmann::json;

    /**
     * @brief Decodes every registered item present in json. Missing keys are left untouched.
     */
    void fromJson(const nlohmann::json& json);

    /**
     * @brief Encodes every registered item and writes it to path as pretty-printed JSON.
     */
    [[nodiscard]] auto saveToFile(const std::filesystem::path& path) const -> bool;

    /**
     * @brief Reads path and decodes every registered item present in it.
     */
    [[nodiscard]] auto loadFromFile(const std::filesystem::path& path) -> bool;

   private:
    struct Item {
        std::string key;
        std::function<nlohmann::json()> encode;
        std::function<void(const nlohmann::json&)> decode;
    };

    std::vector<Item> m_items;
};

}  // namespace Core
