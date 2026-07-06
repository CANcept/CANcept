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

namespace Manipulation {
/**
 * @struct DLCTrigger
 * @brief This trigger activates when the specific dlc is used in the message.
 */
struct DLCTrigger {
    uint8_t dlc;

    /**
     * @brief Checks that the dlc is between 0 and 8.
     * @param dlc the dlc for the trigger
     */
    explicit DLCTrigger(const uint8_t dlc) : dlc(dlc)
    {
        assert(dlc <= 8);
    }
};

}  // namespace Manipulation