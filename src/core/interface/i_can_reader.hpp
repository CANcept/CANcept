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
#include <string>
#include <string_view>
#include <vector>

#include "core/dto/can_dto.hpp"

namespace Core {

/** @brief Discriminates the message type stored in a CAN log file. */
enum class CanFileType { Raw, Dbc };

/**
 * @brief Contract for random-access CAN log readers.
 *
 * Models a seekable file of fixed-stride CAN records. Positions are record
 * indices (not byte offsets), so callers are format-agnostic.
 */
class ICanReader
{
   public:
    /** @brief Closes the file if still open. */
    virtual ~ICanReader() = default;

    /** @brief Opens the file at the given path; returns true on success. */
    [[nodiscard]] virtual auto open(std::string_view path) -> bool = 0;

    /** @brief Closes the file and releases resources. Safe to call when already closed. */
    virtual void close() = 0;

    [[nodiscard]] virtual auto isOpen() const noexcept -> bool = 0;

    /**
     * @brief Returns the message type stored in this file.
     * Valid only after a successful open().
     */
    [[nodiscard]] virtual auto fileType() const noexcept -> CanFileType = 0;

    /**
     * @brief Moves the read cursor to the given record index.
     * @return false if the index is out of range or the file is not open.
     */
    [[nodiscard]] virtual auto seek(uint64_t recordIndex) -> bool = 0;

    /** @brief Returns the current record index. */
    [[nodiscard]] virtual auto tell() const noexcept -> uint64_t = 0;

    /** @brief Returns the total number of records in the file. */
    [[nodiscard]] virtual auto recordCount() const noexcept -> uint64_t = 0;

    /** @brief Returns true when the cursor is at or past the last record. */
    [[nodiscard]] virtual auto eof() const noexcept -> bool = 0;

    /**
     * @brief Reads the next record into @p out and advances the cursor.
     * @return false if eof or the file is not open.
     */
    [[nodiscard]] virtual auto read(RawCanMessage& out) -> bool = 0;
    [[nodiscard]] virtual auto read(DbcCanMessage& out) -> bool = 0;

    /**
     * @brief Returns column header names for all fields in a record, including timestamp.
     */
    [[nodiscard]] virtual auto columnHeaders() const -> std::vector<std::string> = 0;
};

}  // namespace Core