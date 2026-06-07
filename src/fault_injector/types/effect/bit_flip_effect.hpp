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
#include <cassert>
#include <cstdint>

namespace FaultInjector {

/**
 * @struct BitFlipEffect
 * @brief This effect flips the corresponding bit based on 0 LSB, 7 MSB
 */
struct BitFlipEffect {
    uint8_t byteIndex;
    uint8_t bitIndex;

    /**
     * @brief Constructs a BitFlipEffect and makes sure the values are in range.
     * @param byte the byte of the bit to flip
     * @param bit the bit to flip inside that byte
     */
    BitFlipEffect(const uint8_t byte, const uint8_t bit) : byteIndex(byte), bitIndex(bit)
    {
        assert(bit < 8 && byte < 8);
    }
};

}  // namespace FaultInjector