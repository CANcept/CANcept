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

#include "core/dto/can_dto.hpp"

namespace Core {

/**
 * @brief Contract for applying fault injection to outgoing CAN messages.
 *
 * It covers the two meanigful point of injection. before encoding and after encoding.
 */
class IFaultHandler
{
   public:
    virtual ~IFaultHandler() = default;

    /**
     * @brief Applies raw byte-level faults to the given message.
     *
     * @param id   CAN frame identifier
     * @param dlc  Data Length Code (0–8). May be modified by the handler.
     * @param data Raw data bytes. Only bytes [0, dlc) are meaningful.
     */
    virtual void inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data) = 0;

    /**
     * @brief Applies signal-level faults to the given decoded message.
     *
     * @param message The decoded CAN message to mutate in place.
     */
    virtual void inject(DbcCanMessage& message) = 0;
};

}  // namespace Core