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
 * @brief Contract for streaming CAN log writers.
 *
 * Implementations receive decoded or raw CAN messages and persist them in
 * a specific file format. Callers are format-agnostic.
 */
class ICanWriter
{
   public:
    virtual ~ICanWriter() = default;

    virtual void write(const DbcCanMessage& msg) = 0;
    virtual void write(const RawCanMessage& msg) = 0;

    /** @brief Flushes the internal write buffer to disk. */
    virtual void flush() = 0;

    /** @brief Finalizes the file. Safe to call multiple times. */
    virtual void close() = 0;

    [[nodiscard]] virtual bool isOpen() const noexcept = 0;
};

}  // namespace Core