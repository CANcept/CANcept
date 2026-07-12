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
#include <cstdint>

namespace Manipulation {
/**
 * @struct DelayedStrategy
 * @brief This means the frame is delayed by a certain amount.
 */
struct DelayedStrategy {
    uint32_t delayUs;

    /**
     * @brief Constructs the Strategy.
     * @param delayUs the delay
     */
    explicit DelayedStrategy(const uint32_t delayUs) : delayUs(delayUs) {}
};

}  // namespace Manipulation
