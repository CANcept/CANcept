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
#include <array>
#include <chrono>
#include <cstdint>
#include <list>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Core {
struct RawCanMessage {
    std::chrono::nanoseconds receiveTime;
    std::array<char, 8> data;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
    uint8_t dlc{8};      // Data Length Code (0-8 bytes)
};

/** @brief Encodes the raw byte buffer as a JSON array regardless of dlc, for exact round-trips. */
inline void to_json(nlohmann::json& json, const RawCanMessage& message)
{
    json = nlohmann::json{
        {"receiveTimeNs", message.receiveTime.count()},
        {"data", std::vector<uint8_t>(message.data.begin(), message.data.end())},
        {"messageId", message.messageId},
        {"dlc", message.dlc},
    };
}

inline void from_json(const nlohmann::json& json, RawCanMessage& message)
{
    message.receiveTime = std::chrono::nanoseconds(json.at("receiveTimeNs").get<int64_t>());
    const auto bytes = json.at("data").get<std::vector<uint8_t>>();
    for (std::size_t i = 0; i < message.data.size() && i < bytes.size(); ++i)
    {
        message.data[i] = static_cast<char>(bytes[i]);
    }
    message.messageId = json.at("messageId").get<uint16_t>();
    message.dlc = json.at("dlc").get<uint8_t>();
}

struct DbcCanSignal {
    std::string name;
    double value;
};

inline void to_json(nlohmann::json& json, const DbcCanSignal& signal)
{
    json = nlohmann::json{{"name", signal.name}, {"value", signal.value}};
}

inline void from_json(const nlohmann::json& json, DbcCanSignal& signal)
{
    signal.name = json.at("name").get<std::string>();
    signal.value = json.at("value").get<double>();
}

struct DbcCanMessage {
    std::chrono::nanoseconds receiveTime;
    std::vector<DbcCanSignal> signalValues;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
};

inline void to_json(nlohmann::json& json, const DbcCanMessage& message)
{
    json = nlohmann::json{
        {"receiveTimeNs", message.receiveTime.count()},
        {"signalValues", message.signalValues},
        {"messageId", message.messageId},
    };
}

inline void from_json(const nlohmann::json& json, DbcCanMessage& message)
{
    message.receiveTime = std::chrono::nanoseconds(json.at("receiveTimeNs").get<int64_t>());
    message.signalValues = json.at("signalValues").get<std::vector<DbcCanSignal>>();
    message.messageId = json.at("messageId").get<uint16_t>();
}
}  // namespace Core
