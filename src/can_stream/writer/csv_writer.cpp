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

#include "csv_writer.hpp"

#include <algorithm>
#include <iomanip>

namespace CanStream {

CsvWriter::CsvWriter(const std::string_view path, const Core::CanFileType fileType,
                     std::vector<std::string> headers)
    : m_fileType(fileType), m_headers(std::move(headers))
{
    m_file.open(std::string(path));
    if (!m_file.is_open()) return;
    m_open = true;

    for (size_t i = 0; i < m_headers.size(); ++i)
    {
        if (i > 0) m_file << ',';
        m_file << m_headers[i];
    }
    m_file << '\n';
}

void CsvWriter::write(const Core::RawCanMessage& msg)
{
    if (!m_open) return;
    m_file << std::fixed << std::setprecision(9)
           << static_cast<double>(msg.receiveTime.count()) / 1e9;
    m_file << ",1";
    m_file << ",0x" << std::uppercase << std::hex << std::setw(3) << std::setfill('0')
           << msg.messageId;
    m_file << std::dec << ",Rx";
    m_file << ',' << static_cast<int>(msg.dlc);
    m_file << ',' << static_cast<int>(msg.dlc);
    m_file << ',';
    for (int i = 0; i < msg.dlc && i < 8; ++i)
    {
        if (i > 0) m_file << ' ';
        m_file << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(static_cast<unsigned char>(msg.data[i]));
    }
    m_file << '\n';
}

void CsvWriter::write(const Core::DbcCanMessage& msg)
{
    if (!m_open) return;
    m_file << std::fixed << std::setprecision(9)
           << static_cast<double>(msg.receiveTime.count()) / 1e9;

    for (size_t i = 1; i < m_headers.size(); ++i)
    {
        m_file << ',';
        const auto it = std::ranges::find_if(
            msg.signalValues, [&](const auto& sv) { return sv.name == m_headers[i]; });
        if (it != msg.signalValues.end())
            m_file << std::fixed << std::setprecision(3) << it->value;
        else
            m_file << '-';
    }
    m_file << '\n';
}

void CsvWriter::flush()
{
    if (m_open) m_file.flush();
}

void CsvWriter::close()
{
    if (!m_open) return;
    m_file.flush();
    m_file.close();
    m_open = false;
}

auto CsvWriter::isOpen() const noexcept -> bool
{
    return m_open;
}

}  // namespace CanStream