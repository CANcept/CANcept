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

namespace Manipulation {

/**
 * @struct RandomTrigger
 * @brief This trigger activates based on the given probability.
 */
struct RandomTrigger {
    float probability;

    /**
     * @brief Checks that the chance is between 0 and 1.
     * @param chance the chance to hit the trigger
     */
    explicit RandomTrigger(const float chance) : probability(chance)
    {
        assert(chance >= 0.0f && chance <= 1.0f);
    }
};

}  // namespace Manipulation
