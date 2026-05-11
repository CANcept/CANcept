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

#include <fstream>
#include <string>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_can_reader.hpp"

namespace CanStream {

/**
 * @brief MDF4 reader implementing ICanReader for both RAW and DBC session files.
 *
 * Call open() to parse the header; the file type (Raw/Dbc) is detected automatically.
 * Use seek() + read() for random record access, or iterate with read() + eof().
 */
class Mdf4Reader final : public Core::ICanReader
{
   public:
    Mdf4Reader() = default;
    ~Mdf4Reader() override;

    Mdf4Reader(const Mdf4Reader&) = delete;
    auto operator=(const Mdf4Reader&) -> Mdf4Reader& = delete;

    /** @brief Opens and parses the MDF4 file at path; returns false on failure. */
    [[nodiscard]] auto open(std::string_view path) -> bool override;

    /** @brief Closes the file and resets all state. Safe to call when already closed. */
    void close() override;

    [[nodiscard]] auto isOpen() const noexcept -> bool override;

    /** @brief Returns the session type detected from the file header. Valid after open(). */
    [[nodiscard]] auto fileType() const noexcept -> Core::CanFileType override;

    /** @brief Moves the cursor to recordIndex; returns false if out of range. */
    [[nodiscard]] auto seek(uint64_t recordIndex) -> bool override;

    [[nodiscard]] auto tell() const noexcept -> uint64_t override;
    [[nodiscard]] auto recordCount() const noexcept -> uint64_t override;
    [[nodiscard]] auto eof() const noexcept -> bool override;

    /** @brief Reads the next record into out and advances the cursor. Returns false if eof or wrong
     * type. */
    [[nodiscard]] auto read(Core::RawCanMessage& out) -> bool override;
    [[nodiscard]] auto read(Core::DbcCanMessage& out) -> bool override;
    [[nodiscard]] auto columnHeaders() const -> std::vector<std::string> override;

   private:
    auto parseHeader() -> bool;
    auto parseDbcGroups(uint64_t firstCg) -> bool;
    auto buildDbcIndex(uint64_t dtFirst) -> bool;
    auto readTxText(uint64_t txOffset) -> std::string;

    auto readAt(uint64_t offset, void* buf, size_t n) -> bool;
    auto readU8At(uint64_t offset, uint8_t& v) -> bool;
    auto readU32At(uint64_t offset, uint32_t& v) -> bool;
    auto readU64At(uint64_t offset, uint64_t& v) -> bool;

    std::ifstream m_file;
    Core::CanFileType m_fileType{Core::CanFileType::Raw};
    uint64_t m_cursor{0};
    uint64_t m_recordCount{0};
    bool m_open{false};

    uint64_t m_dtDataStart{0};
    uint64_t m_startNs{0};

    std::vector<std::string> m_columnHeaders;

    struct DbcGroup {
        uint8_t recordId;
        uint32_t payloadBytes;
        uint16_t msgId;
        std::vector<std::string> signalNames;
    };
    std::vector<DbcGroup> m_dbcGroups;
    std::vector<uint64_t> m_dbcOffsets;
    std::vector<uint8_t> m_dbcGroupIdx;
};

}  // namespace CanStream