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
 * @brief ICanReader implementation for CSV files produced by CsvWriter / CanConverter.
 *
 * Records are streamed line-by-line so arbitrarily large files are supported without
 * loading them into memory. open() performs a one-time sequential line count so that
 * recordCount() returns an accurate value for progress tracking.
 *
 * RAW files are detected by the presence of an "ID" column in the header row;
 * everything else is treated as DBC.
 */
class CsvReader final : public Core::ICanReader
{
   public:
    CsvReader() = default;
    ~CsvReader() override = default;

    CsvReader(const CsvReader&) = delete;
    auto operator=(const CsvReader&) -> CsvReader& = delete;

    [[nodiscard]] auto open(std::string_view path) -> bool override;
    void close() override;
    [[nodiscard]] auto isOpen() const noexcept -> bool override;
    [[nodiscard]] auto fileType() const noexcept -> Core::CanFileType override;
    [[nodiscard]] auto seek(uint64_t recordIndex) -> bool override;
    [[nodiscard]] auto tell() const noexcept -> uint64_t override;

    [[nodiscard]] auto recordCount() const noexcept -> uint64_t override;

    [[nodiscard]] auto eof() const noexcept -> bool override;
    [[nodiscard]] auto read(Core::RawCanMessage& out) -> bool override;
    [[nodiscard]] auto read(Core::DbcCanMessage& out) -> bool override;
    [[nodiscard]] auto columnHeaders() const -> std::vector<std::string> override;

   private:
    auto advanceLookahead() -> void;

    std::ifstream m_file;
    std::streampos m_dataStart{};
    std::string m_nextLine;

    Core::CanFileType m_fileType{Core::CanFileType::Raw};
    bool m_open{false};
    bool m_eof{true};
    uint64_t m_cursor{0};
    uint64_t m_recordCount{0};

    std::vector<std::string> m_columnHeaders;

    // Column indices resolved from the header row (RAW only)
    int m_colId{2};
    int m_colDlc{4};
    int m_colData{6};
};

}  // namespace CanStream