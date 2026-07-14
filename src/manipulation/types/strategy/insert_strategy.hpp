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
#include <optional>

#include "core/dto/can_dto.hpp"

namespace Manipulation {
/**
 * @struct DbcInsertStrategy
 * @brief This means a brand-new DBC message is sent after a delay, in addition to the
 * frame that triggered it.
 */
struct DbcInsertStrategy {
    uint32_t delayUs;
    /** @brief The message to insert. std::nullopt means a copy of the frame that
     * triggered this manipulation is inserted instead of a fixed, configured message. */
    std::optional<Core::DbcCanMessage> message;
};

}  // namespace Manipulation
