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

#include "can_converter.hpp"

#include <filesystem>
#include <unordered_map>

#include "can_stream/reader/mdf4_reader.hpp"
#include "can_stream/writer/csv_writer.hpp"

namespace CanStream {

namespace {

auto toDisplayName(const std::string& raw) -> std::string
{
    static const std::unordered_map<std::string, std::string> map = {
        {"t", "Time (s)"},
        {"CAN_DataFrame.BusChannel", "Channel"},
        {"CAN_DataFrame.ID", "ID"},
        {"CAN_DataFrame.Dir", "Direction"},
        {"CAN_DataFrame.DLC", "DLC"},
        {"CAN_DataFrame.DataLength", "Data Length"},
        {"CAN_DataFrame.DataBytes", "Data"},
    };
    const auto it = map.find(raw);
    return it != map.end() ? it->second : raw;
}

}  // namespace

CanConverter::CanConverter(std::string sourcePath, std::string targetPath, ExportType format)
    : m_source(std::move(sourcePath)), m_target(std::move(targetPath)), m_format(format)
{
}

auto CanConverter::convert() -> std::string
{
    if (m_format == ExportType::Mdf4)
    {
        std::error_code ec;
        std::filesystem::copy_file(m_source, m_target,
                                   std::filesystem::copy_options::overwrite_existing, ec);
        return ec ? std::string{} : m_target;
    }

    Mdf4Reader reader;
    if (!reader.open(m_source)) return {};

    std::vector<std::string> headers;
    for (const auto& h : reader.columnHeaders()) headers.push_back(toDisplayName(h));

    CsvWriter writer(m_target, reader.fileType(), std::move(headers));
    if (!writer.isOpen()) return {};

    if (reader.fileType() == Core::CanFileType::Raw)
    {
        Core::RawCanMessage msg;
        while (reader.read(msg)) writer.write(msg);
    } else
    {
        Core::DbcCanMessage msg;
        while (reader.read(msg)) writer.write(msg);
    }

    writer.close();
    return m_target;
}

}  // namespace CanStream