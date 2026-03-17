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
#include <ctime>
#include <string>
#include <unordered_map>

#include "core/dto/can_dto.hpp"
#include "event.hpp"
namespace Core {
/**
 * @brief Structure of the received can event when a can message is received and used in raw form
 */
struct ReceivedCanRawEvent final : public Event {
    RawCanMessage canMessage;

    explicit ReceivedCanRawEvent(const RawCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the received can event when a can message is received and used in dbc decoded
 * form
 */
struct ReceivedCanDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit ReceivedCanDbcEvent(const DbcCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the send can event, when an already encoded message should be sent
 */
struct SendCanMessageRawEvent final : public Event {
    RawCanMessage canMessage;

    explicit SendCanMessageRawEvent(const RawCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the send can event, when a message should be sent based on the current DBC
 * config
 */
struct SendCanMessageDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit SendCanMessageDbcEvent(const DbcCanMessage& canMessage) : canMessage(canMessage){};
};
};  // namespace Core
