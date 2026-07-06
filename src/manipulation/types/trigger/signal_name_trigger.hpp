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
 * @struct SignalNameTrigger
 * @brief This trigger activates based on the signal name of a message.
 */
struct SignalNameTrigger {
    std::string signal_name;

    /**
     * @brief Constructs a signal name trigger.
     * @param signal_name the signal name for the trigger
     */
    explicit SignalNameTrigger(std::string signal_name) : signal_name(std::move(signal_name)) {}
};

}  // namespace Manipulation