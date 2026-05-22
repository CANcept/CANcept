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

#include "csv_reader.hpp"

#include <charconv>
#include <sstream>

namespace CanStream {

namespace {

auto splitLine(const std::string& line) -> std::vector<std::string>
{
    std::vector<std::string> fields;
    std::istringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) fields.push_back(std::move(field));
    return fields;
}

auto parseHexId(const std::string& s) -> uint16_t
{
    const char* p = s.data();
    const char* end = p + s.size();
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) p += 2;
    uint32_t val = 0;
    std::from_chars(p, end, val, 16);
    return static_cast<uint16_t>(val & 0x7FFu);
}

auto parseTimestampNs(const std::string& s) -> std::chrono::nanoseconds
{
    const auto dotPos = s.find('.');
    int64_t intPart = 0;
    std::from_chars(s.data(), s.data() + (dotPos == std::string::npos ? s.size() : dotPos),
                    intPart);
    int64_t fracPart = 0;
    if (dotPos != std::string::npos)
    {
        const char* fracStart = s.data() + dotPos + 1;
        const char* fracEnd = s.data() + s.size();
        size_t fracLen = static_cast<size_t>(fracEnd - fracStart);
        std::from_chars(fracStart, fracEnd, fracPart);
        // normalize to 9 digits (nanoseconds)
        for (size_t i = fracLen; i < 9; ++i) fracPart *= 10;
        for (size_t i = fracLen; i > 9; --i) fracPart /= 10;
    }
    return std::chrono::nanoseconds(intPart * 1'000'000'000LL + fracPart);
}

}  // namespace

void CsvReader::advanceLookahead()
{
    do
    {
        m_eof = !std::getline(m_file, m_nextLine);
    } while (!m_eof && m_nextLine.empty());
}

auto CsvReader::open(std::string_view path) -> bool
{
    close();
    m_file.open(std::string(path));
    if (!m_file.is_open()) return false;

    std::string header;
    if (!std::getline(m_file, header))
    {
        close();
        return false;
    }

    m_columnHeaders = splitLine(header);

    // Resolve column indices by name and detect file type
    m_fileType = Core::CanFileType::Dbc;
    for (size_t i = 0; i < m_columnHeaders.size(); ++i)
    {
        const auto& h = m_columnHeaders[i];
        if (h == "ID")
        {
            m_fileType = Core::CanFileType::Raw;
            m_colId = static_cast<int>(i);
        } else if (h == "DLC")
        {
            m_colDlc = static_cast<int>(i);
        } else if (h == "Data")
        {
            m_colData = static_cast<int>(i);
        } else if (h == "MsgID")
        {
            m_colMsgId = static_cast<int>(i);
        }
    }

    m_dataStart = m_file.tellg();

    m_recordCount = 0;
    std::string countLine;
    while (std::getline(m_file, countLine))
    {
        if (!countLine.empty()) ++m_recordCount;
    }

    m_file.clear();
    m_file.seekg(m_dataStart);
    m_cursor = 0;
    m_open = true;
    advanceLookahead();
    return true;
}

void CsvReader::close()
{
    if (m_file.is_open()) m_file.close();
    m_open = false;
    m_eof = true;
    m_cursor = 0;
    m_recordCount = 0;
    m_colMsgId = -1;
    m_nextLine.clear();
    m_columnHeaders.clear();
}

auto CsvReader::isOpen() const noexcept -> bool
{
    return m_open;
}
auto CsvReader::fileType() const noexcept -> Core::CanFileType
{
    return m_fileType;
}
auto CsvReader::tell() const noexcept -> uint64_t
{
    return m_cursor;
}
auto CsvReader::recordCount() const noexcept -> uint64_t
{
    return m_recordCount;
}
auto CsvReader::eof() const noexcept -> bool
{
    return !m_open || m_eof;
}
auto CsvReader::columnHeaders() const -> std::vector<std::string>
{
    return m_columnHeaders;
}

auto CsvReader::seek(uint64_t recordIndex) -> bool
{
    if (!m_open) return false;

    if (recordIndex < m_cursor)
    {
        m_file.clear();
        m_file.seekg(m_dataStart);
        m_cursor = 0;
        advanceLookahead();
    }

    while (m_cursor < recordIndex && !m_eof)
    {
        advanceLookahead();
        ++m_cursor;
    }

    return m_cursor == recordIndex;
}

auto CsvReader::read(Core::RawCanMessage& out) -> bool
{
    if (!m_open || m_eof || m_fileType != Core::CanFileType::Raw) return false;

    const auto fields = splitLine(m_nextLine);
    advanceLookahead();
    ++m_cursor;

    if (static_cast<int>(fields.size()) <= m_colData) return false;

    out.receiveTime = parseTimestampNs(fields[0]);
    out.messageId = parseHexId(fields[m_colId]);

    out.dlc = 0;
    if (m_colDlc < static_cast<int>(fields.size()))
    {
        uint8_t dlc = 0;
        std::from_chars(fields[m_colDlc].data(), fields[m_colDlc].data() + fields[m_colDlc].size(),
                        dlc);
        out.dlc = dlc;
    }

    out.data.fill(0);
    const std::string& dataField = fields[m_colData];
    int byteIdx = 0;
    size_t pos = 0;
    while (byteIdx < 8 && pos < dataField.size())
    {
        while (pos < dataField.size() && dataField[pos] == ' ') ++pos;
        if (pos + 2 > dataField.size()) break;
        uint8_t byte = 0;
        std::from_chars(dataField.data() + pos, dataField.data() + pos + 2, byte, 16);
        out.data[byteIdx++] = static_cast<char>(byte);
        pos += 2;
    }

    return true;
}

auto CsvReader::read(Core::DbcCanMessage& out) -> bool
{
    if (!m_open || m_eof || m_fileType != Core::CanFileType::Dbc) return false;

    const auto fields = splitLine(m_nextLine);
    advanceLookahead();
    ++m_cursor;

    if (fields.empty()) return false;

    out.receiveTime = parseTimestampNs(fields[0]);
    out.messageId = 0;
    if (m_colMsgId >= 0 && m_colMsgId < static_cast<int>(fields.size()))
        out.messageId = parseHexId(fields[m_colMsgId]);

    out.signalValues.clear();

    for (size_t i = 1; i < m_columnHeaders.size() && i < fields.size(); ++i)
    {
        if (static_cast<int>(i) == m_colMsgId) continue;
        const std::string& val = fields[i];
        if (val.empty() || val == "-") continue;
        try
        {
            out.signalValues.push_back({.name = m_columnHeaders[i], .value = std::stod(val)});
        } catch (...)
        {
        }
    }

    return true;
}

}  // namespace CanStream