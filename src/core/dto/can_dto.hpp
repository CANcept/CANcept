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
#include <list>
#include <string>

namespace Core {
struct RawCanMessage {
    std::chrono::milliseconds receiveTime;
    std::array<char, 8> data;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
    uint8_t dlc{8};      // Data Length Code (0-8 bytes)
};
struct DbcCanSignal {
    std::string name;
    double value;
};
struct DbcCanMessage {
    std::chrono::milliseconds receiveTime;
    std::list<DbcCanSignal> signalValues;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
};
}  // namespace Core
