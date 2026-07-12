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
#include <string>
#include <utility>

namespace Manipulation {

/**
 * @struct ClampEffect
 * @brief This effect clamps a signal value into a defined range.
 */
struct ClampEffect {
    std::string signal_name;
    double min_value;
    double max_value;

    ClampEffect(std::string signal_name, const double min_value, const double max_value)
        : signal_name(std::move(signal_name)), min_value(min_value), max_value(max_value)
    {
    }
};

}  // namespace Manipulation