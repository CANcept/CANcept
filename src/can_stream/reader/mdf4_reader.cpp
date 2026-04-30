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

#include "mdf4_reader.hpp"

#include <array>
#include <cstring>
#include <unordered_map>

namespace CanStream {

Mdf4Reader::~Mdf4Reader()
{
    close();
}

auto Mdf4Reader::open(std::string_view path) -> bool
{
    close();
    m_file.open(std::string(path), std::ios::binary);
    if (!m_file.is_open()) return false;
    if (!parseHeader())
    {
        close();
        return false;
    }
    m_open = true;
    return true;
}

void Mdf4Reader::close()
{
    if (m_file.is_open()) m_file.close();
    m_open = false;
    m_cursor = 0;
    m_recordCount = 0;
    m_dtDataStart = 0;
    m_startNs = 0;
    m_dbcGroups.clear();
    m_dbcOffsets.clear();
    m_dbcGroupIdx.clear();
    m_columnHeaders.clear();
}

auto Mdf4Reader::isOpen() const noexcept -> bool
{
    return m_open;
}

auto Mdf4Reader::fileType() const noexcept -> Core::CanFileType
{
    return m_fileType;
}

auto Mdf4Reader::seek(uint64_t recordIndex) -> bool
{
    if (!m_open || recordIndex > m_recordCount) return false;
    m_cursor = recordIndex;
    return true;
}

auto Mdf4Reader::tell() const noexcept -> uint64_t
{
    return m_cursor;
}

auto Mdf4Reader::recordCount() const noexcept -> uint64_t
{
    return m_recordCount;
}

auto Mdf4Reader::eof() const noexcept -> bool
{
    return m_cursor >= m_recordCount;
}

auto Mdf4Reader::read(Core::RawCanMessage& out) -> bool
{
    if (!m_open || eof() || m_fileType != Core::CanFileType::Raw) return false;

    // RAW record: [f64 t_s][u8 BusChannel][u32 ID][u8 Dir][u8 DLC][u8 DataLength][u8×8 data]
    std::array<uint8_t, 24> buf{};
    if (!readAt(m_dtDataStart + m_cursor * 24u, buf.data(), 24)) return false;

    double relTimeSec;
    std::memcpy(&relTimeSec, buf.data(), 8);

    uint32_t id;
    std::memcpy(&id, buf.data() + 9, 4);

    out.receiveTime = std::chrono::nanoseconds(m_startNs + static_cast<uint64_t>(relTimeSec * 1e9));
    out.messageId = static_cast<uint16_t>(id & 0x7FFu);
    out.dlc = buf[14];
    std::memcpy(out.data.data(), buf.data() + 16, 8);

    ++m_cursor;
    return true;
}

auto Mdf4Reader::read(Core::DbcCanMessage& out) -> bool
{
    if (!m_open || eof() || m_fileType != Core::CanFileType::Dbc) return false;

    const uint64_t pos = m_dbcOffsets[m_cursor];
    const DbcGroup& group = m_dbcGroups[m_dbcGroupIdx[m_cursor]];

    // DBC record (after rec_id): [f64 t_s][f64 sig0]...[f64 sigN-1]
    std::vector<uint8_t> buf(group.payloadBytes);
    if (!readAt(pos + 1, buf.data(), group.payloadBytes)) return false;

    double relTimeSec;
    std::memcpy(&relTimeSec, buf.data(), 8);

    out.receiveTime = std::chrono::nanoseconds(m_startNs + static_cast<uint64_t>(relTimeSec * 1e9));
    out.messageId = group.msgId;
    out.signalValues.clear();
    out.signalValues.reserve(group.signalNames.size());

    for (size_t i = 0; i < group.signalNames.size(); ++i)
    {
        double value;
        std::memcpy(&value, buf.data() + 8 + i * 8, 8);
        out.signalValues.push_back({.name = group.signalNames[i], .value = value});
    }

    ++m_cursor;
    return true;
}

auto Mdf4Reader::columnHeaders() const -> std::vector<std::string>
{
    return m_columnHeaders;
}

auto Mdf4Reader::parseHeader() -> bool
{
    // Verify IDBLOCK magic at byte 0
    std::array<char, 8> magic{};
    m_file.read(magic.data(), 8);
    if (m_file.fail() || std::memcmp(magic.data(), "MDF     ", 8) != 0) return false;

    // HDBLOCK sits at offset 64; hd_dg_first is its first link at HDBLOCK+24
    uint64_t dgOffset;
    if (!readU64At(64 + 24, dgOffset)) return false;

    // hd_start_time_ns is at HDBLOCK+24+48 (after header + 6 links)
    if (!readU64At(64 + 24 + 48, m_startNs)) return false;

    // DGBLOCK: dg_cg_first (+32), dg_dt_first (+40), dg_rec_id_size (+56)
    uint64_t cgFirst, dtFirst;
    uint8_t recIdSize;
    if (!readU64At(dgOffset + 32, cgFirst)) return false;
    if (!readU64At(dgOffset + 40, dtFirst)) return false;
    if (!readU8At(dgOffset + 56, recIdSize)) return false;

    m_fileType = (recIdSize == 0) ? Core::CanFileType::Raw : Core::CanFileType::Dbc;

    if (m_fileType == Core::CanFileType::Raw)
    {
        m_dtDataStart = dtFirst + 24;
        uint64_t dtLength;
        if (!readU64At(dtFirst + 8, dtLength)) return false;
        m_recordCount = (dtLength - 24) / 24;

        uint64_t cnFirst;
        if (!readU64At(cgFirst + 32, cnFirst)) return false;
        uint64_t cnOffset = cnFirst;
        while (cnOffset != 0)
        {
            uint64_t cnNext, compositionOffset, txNameOffset;
            if (!readU64At(cnOffset + 24, cnNext)) return false;
            if (!readU64At(cnOffset + 32, compositionOffset)) return false;
            if (!readU64At(cnOffset + 40, txNameOffset)) return false;

            if (compositionOffset != 0)
            {
                // Sub-channels are linked via cn_cn_next starting at the composition pointer
                uint64_t sub = compositionOffset;
                while (sub != 0)
                {
                    uint64_t subNext, subComp, subTx;
                    if (!readU64At(sub + 24, subNext)) return false;
                    if (!readU64At(sub + 32, subComp)) return false;
                    if (!readU64At(sub + 40, subTx)) return false;
                    if (subComp == 0 && subTx != 0) m_columnHeaders.push_back(readTxText(subTx));
                    sub = subNext;
                }
            } else if (txNameOffset != 0)
            {
                m_columnHeaders.push_back(readTxText(txNameOffset));
            }

            cnOffset = cnNext;
        }
        return true;
    }

    return parseDbcGroups(cgFirst) && buildDbcIndex(dtFirst);
}

auto Mdf4Reader::parseDbcGroups(uint64_t firstCg) -> bool
{
    uint64_t cgOffset = firstCg;
    while (cgOffset != 0)
    {
        uint64_t cgNext, cnFirst, recordId;
        uint32_t dataBytes;

        if (!readU64At(cgOffset + 24, cgNext)) return false;     // cg_cg_next
        if (!readU64At(cgOffset + 32, cnFirst)) return false;    // cg_cn_first
        if (!readU64At(cgOffset + 72, recordId)) return false;   // cg_record_id
        if (!readU32At(cgOffset + 96, dataBytes)) return false;  // cg_data_bytes

        DbcGroup group;
        group.recordId = static_cast<uint8_t>(recordId);
        group.payloadBytes = dataBytes;

        uint64_t mdOffset;
        if (readU64At(cgOffset + 64, mdOffset) && mdOffset != 0)
        {
            const std::string xml = readTxText(mdOffset);
            const auto start = xml.find("<id>0x");
            const auto end = xml.find("</id>");
            if (start != std::string::npos && end != std::string::npos)
                group.msgId = static_cast<uint16_t>(
                    std::stoul(xml.substr(start + 6, end - start - 6), nullptr, 16));
            else
                group.msgId = static_cast<uint16_t>(recordId);
        } else
        {
            group.msgId = static_cast<uint16_t>(recordId);
        }

        // Walk CN chain - collect signal names and build column headers (dedup across groups)
        uint64_t cnOffset = cnFirst;
        while (cnOffset != 0)
        {
            uint64_t cnNext, txNameOffset;
            uint8_t syncType;

            if (!readU64At(cnOffset + 24, cnNext)) return false;        // cn_cn_next
            if (!readU64At(cnOffset + 40, txNameOffset)) return false;  // cn_tx_name (link[2])
            if (!readU8At(cnOffset + 89, syncType)) return false;       // cn_sync_type

            if (txNameOffset != 0)
            {
                std::string name = readTxText(txNameOffset);
                if (syncType != 1) group.signalNames.push_back(name);
                // Add to column headers only if not already present (deduplicates "t")
                if (std::ranges::find(m_columnHeaders, name) == m_columnHeaders.end())
                    m_columnHeaders.push_back(std::move(name));
            }

            cnOffset = cnNext;
        }

        m_dbcGroups.push_back(std::move(group));
        cgOffset = cgNext;
    }
    return !m_dbcGroups.empty();
}

auto Mdf4Reader::buildDbcIndex(const uint64_t dtFirst) -> bool
{
    std::unordered_map<uint8_t, uint8_t> groupByRecId;
    for (size_t i = 0; i < m_dbcGroups.size(); ++i)
        groupByRecId[m_dbcGroups[i].recordId] = static_cast<uint8_t>(i);

    uint64_t dtLength;
    if (!readU64At(dtFirst + 8, dtLength)) return false;

    const uint64_t dtDataEnd = dtFirst + dtLength;
    uint64_t pos = dtFirst + 24;

    // Stream forward through the DT block; skip payloads with relative seeks
    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(pos));

    while (pos < dtDataEnd)
    {
        char recIdByte;
        m_file.read(&recIdByte, 1);
        if (m_file.fail()) break;

        const auto recId = static_cast<uint8_t>(recIdByte);
        auto it = groupByRecId.find(recId);
        if (it == groupByRecId.end()) return false;

        m_dbcOffsets.push_back(pos);
        m_dbcGroupIdx.push_back(it->second);

        const uint32_t payloadBytes = m_dbcGroups[it->second].payloadBytes;
        m_file.seekg(static_cast<std::streamoff>(payloadBytes), std::ios::cur);
        pos += 1 + payloadBytes;
    }

    m_recordCount = m_dbcOffsets.size();
    return true;
}

auto Mdf4Reader::readTxText(uint64_t txOffset) -> std::string
{
    std::string result;
    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(txOffset + 24));
    if (!m_file.fail()) std::getline(m_file, result, '\0');
    return result;
}

auto Mdf4Reader::readAt(uint64_t offset, void* buf, size_t n) -> bool
{
    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(offset));
    m_file.read(static_cast<char*>(buf), static_cast<std::streamsize>(n));
    return !m_file.fail();
}

auto Mdf4Reader::readU8At(const uint64_t offset, uint8_t& v) -> bool
{
    return readAt(offset, &v, 1);
}

auto Mdf4Reader::readU32At(const uint64_t offset, uint32_t& v) -> bool
{
    return readAt(offset, &v, 4);
}

auto Mdf4Reader::readU64At(const uint64_t offset, uint64_t& v) -> bool
{
    return readAt(offset, &v, 8);
}

}  // namespace CanStream