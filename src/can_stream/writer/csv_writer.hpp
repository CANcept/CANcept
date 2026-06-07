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
#include <string_view>
#include <vector>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_can_reader.hpp"
#include "core/interface/i_can_writer.hpp"

namespace CanStream {

/**
 * @brief ICanWriter implementation that serialises CAN records to CSV.
 *
 * The header row is written on open. Column names and file type are provided
 * by ConverterBuilder, which reads them from the source file before
 * constructing this writer.
 *
 * For DBC files the supplied headers (after the leading "Time" column) are
 * matched against SignalValue::name so sparse rows are handled gracefully when
 * a message does not carry every signal.
 */
class CsvWriter final : public Core::ICanWriter
{
   public:
    CsvWriter(std::string_view path, Core::CanFileType fileType, std::vector<std::string> headers);

    void write(const Core::RawCanMessage& msg) override;
    void write(const Core::DbcCanMessage& msg) override;
    void flush() override;
    void close() override;
    [[nodiscard]] auto isOpen() const noexcept -> bool override;

   private:
    std::ofstream m_file;
    Core::CanFileType m_fileType;
    std::vector<std::string> m_headers;
    bool m_open{false};
};

}  // namespace CanStream