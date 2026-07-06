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

#include <chrono>

#include "core/dto/can_dto.hpp"

namespace Core {

/**
 * @brief Contract for applying manipulation to outgoing CAN messages.
 *
 * It covers the two meaningful points of injection: before encoding and after encoding.
 * Per-frame state is accumulated across inject() calls and consumed by evaluate().
 */
class IManipulationHandler
{
   public:
    /**
     * @brief Result of how the frame should be sent or not.
     */
    struct FrameResult {
        bool drop;
        std::chrono::microseconds delayOffset;
    };

    virtual ~IManipulationHandler() = default;

    /**
     * @brief Applies raw byte-level manipulations to the given message.
     *
     * Also records the strategy of every manipulation that fires into per-frame state.
     *
     * @param id   CAN frame identifier
     * @param dlc  Data Length Code (0–8). May be modified by the handler.
     * @param data Raw data bytes. Only bytes [0, dlc) are meaningful.
     */
    virtual void inject(uint16_t& id, uint8_t& dlc, std::array<char, 8>& data) = 0;

    /**
     * @brief Applies signal-level manipulations to the given decoded message.
     *
     * Also records the strategy of every manipulation that fires into per-frame state.
     *
     * @param message The decoded CAN message to mutate in place.
     */
    virtual void inject(DbcCanMessage& message) = 0;

    /**
     * @brief Returns the aggregated timing decision for the current frame and resets per-frame
     * state.
     */
    virtual auto evaluate() -> FrameResult = 0;
};

}  // namespace Core