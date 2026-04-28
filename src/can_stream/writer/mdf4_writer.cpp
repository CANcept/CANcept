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

#include "mdf4_writer.hpp"

#include <stdexcept>

#include "can_stream/constants.hpp"
#include "mdf4_helper.hpp"

namespace CanStream {

void Mdf4Writer::writeIdBlock()
{
    // IDBLOCK has no standard block header — fixed 64-byte layout
    wIdent(m_file, "MDF     ", 8);
    wIdent(m_file, "4.10    ", 8);
    wIdent(m_file, "CANcept", 8);
    wU32(m_file, 0);                              // reserved
    wU16(m_file, 410);                            // id_ver
    wU16(m_file, 0);                              // reserved
    wU16(m_file, 0);                              // id_unfin_flags
    wU16(m_file, 0);                              // id_custom_unfin_flags
    for (int i = 0; i < 28; ++i) wU8(m_file, 0);  // reserved
    // 8+8+8+4+2+2+2+2+28 = 64 ✓
}

auto Mdf4Writer::writeHdBlock(uint64_t dgOffset, uint64_t fhOffset, uint64_t startNs) -> uint64_t
{
    // 24 (header) + 6×8 (links) + 32 (data) = 104 bytes
    wBlockHdr(m_file, "##HD", 104, 6);
    wU64(m_file, dgOffset);  // hd_dg_first
    wU64(m_file, fhOffset);  // hd_fh_first
    wU64(m_file, 0);         // hd_ch_first
    wU64(m_file, 0);         // hd_at_first
    wU64(m_file, 0);         // hd_ev_first
    wU64(m_file, 0);         // hd_md_comment
    wU64(m_file, startNs);   // hd_start_time_ns
    wI16(m_file, 0);         // hd_tz_offset_min
    wI16(m_file, 0);         // hd_dst_offset_min
    wU8(m_file, 0);          // hd_time_flags
    wU8(m_file, 0);          // hd_time_class
    wU8(m_file, 0);          // hd_flags
    wU8(m_file, 0);          // reserved
    wF64(m_file, 0.0);       // hd_start_angle_rad
    wF64(m_file, 0.0);       // hd_start_distance_m
    return 64;
}

auto Mdf4Writer::writeFhBlock(uint64_t startNs) -> uint64_t
{
    // 24 (header) + 2×8 (links) + 16 (data) = 56 bytes
    wBlockHdr(m_file, "##FH", 56, 2);
    wU64(m_file, 0);        // fh_fh_next
    wU64(m_file, 0);        // fh_md_comment
    wU64(m_file, startNs);  // fh_time_ns
    wI16(m_file, 0);        // fh_tz_offset_min
    wI16(m_file, 0);        // fh_dst_offset_min
    wU8(m_file, 0);         // fh_time_flags
    wU8(m_file, 0);         // reserved
    wU16(m_file, 0);        // reserved
    return currentOffset() - 56;
}

auto Mdf4Writer::writeDgBlock(uint64_t firstCgOffset, uint64_t dtOffset,
                              uint8_t recIdSize) -> uint64_t
{
    // 24 (header) + 4×8 (links) + 8 (data) = 64 bytes
    wBlockHdr(m_file, "##DG", 64, 4);
    wU64(m_file, 0);                             // dg_dg_next
    wU64(m_file, firstCgOffset);                 // dg_cg_first
    wU64(m_file, dtOffset);                      // dg_dt_first
    wU64(m_file, 0);                             // dg_md_comment
    wU8(m_file, recIdSize);                      // dg_rec_id_size
    for (int i = 0; i < 7; ++i) wU8(m_file, 0);  // reserved
    return currentOffset() - 64;
}

auto Mdf4Writer::writeCgBlock(uint64_t nextCgOffset, uint64_t firstCnOffset, uint64_t txAcqName,
                              uint64_t recordId, uint32_t dataBytes, uint16_t cgFlags,
                              uint64_t siOffset, uint16_t pathSeparator) -> uint64_t
{
    // 24 (header) + 6×8 (links) + 32 (data) = 104 bytes
    wBlockHdr(m_file, "##CG", 104, 6);
    wU64(m_file, nextCgOffset);   // cg_cg_next
    wU64(m_file, firstCnOffset);  // cg_cn_first
    wU64(m_file, txAcqName);      // cg_tx_acqname
    wU64(m_file, siOffset);       // cg_si_acqsource
    wU64(m_file, 0);              // cg_sr_first
    wU64(m_file, 0);              // cg_md_comment
    wU64(m_file, recordId);       // cg_record_id
    // cg_cycle_count — patched on close()
    m_cgCyclePos.push_back(static_cast<std::streampos>(currentOffset()));
    wU64(m_file, 0);
    wU16(m_file, cgFlags);        // cg_flags
    wU16(m_file, pathSeparator);  // cg_path_separator
    wU32(m_file, 0);              // reserved
    wU32(m_file, dataBytes);      // cg_data_bytes
    wU32(m_file, 0);              // cg_inval_bytes
    return currentOffset() - 104;
}

auto Mdf4Writer::writeSiBlock(const uint8_t busType) -> uint64_t
{
    // 24 (header) + 3×8 (links) + 8 (data) = 56 bytes
    const uint64_t offset = currentOffset();
    wBlockHdr(m_file, "##SI", 56, 3);
    wU64(m_file, 0);                             // si_tx_name
    wU64(m_file, 0);                             // si_tx_path
    wU64(m_file, 0);                             // si_md_comment
    wU8(m_file, 2);                              // si_type = 2 (BUS)
    wU8(m_file, busType);                        // si_bus_type
    wU8(m_file, 0);                              // si_flags
    for (int i = 0; i < 5; ++i) wU8(m_file, 0);  // reserved
    return offset;
}

auto Mdf4Writer::writeCnBlock(uint64_t nextCnOffset, uint64_t txNameOffset, uint64_t txUnitOffset,
                              uint8_t channelType, uint8_t syncType, uint8_t dataType,
                              uint32_t byteOffset, uint32_t bitCount,
                              uint64_t compositionOffset) -> uint64_t
{
    // 24 (header) + 8×8 (links) + 72 (data) = 160 bytes
    wBlockHdr(m_file, "##CN", 160, 8);
    wU64(m_file, nextCnOffset);       // cn_cn_next
    wU64(m_file, compositionOffset);  // cn_composition
    wU64(m_file, txNameOffset);       // cn_tx_name
    wU64(m_file, 0);                  // cn_si_source
    wU64(m_file, 0);                  // cn_cc_conversion (0 = identity)
    wU64(m_file, 0);                  // cn_data
    wU64(m_file, txUnitOffset);       // cn_md_unit
    wU64(m_file, 0);                  // cn_md_comment
    wU8(m_file, channelType);         // cn_channel_type
    wU8(m_file, syncType);            // cn_sync_type
    wU8(m_file, dataType);            // cn_data_type
    wU8(m_file, 0);                   // cn_bit_offset
    wU32(m_file, byteOffset);         // cn_byte_offset
    wU32(m_file, bitCount);           // cn_bit_count
    wU32(m_file, 0);                  // cn_flags
    wU32(m_file, 0);                  // cn_inval_bit_pos
    wU8(m_file, 0xFF);                // cn_precision (0xFF = unrestricted)
    wU8(m_file, 0);                   // reserved
    wU16(m_file, 0);                  // cn_val_range_n
    wF64(m_file, 0.0);                // cn_val_range_min
    wF64(m_file, 0.0);                // cn_val_range_max
    wF64(m_file, 0.0);                // cn_limit_min
    wF64(m_file, 0.0);                // cn_limit_max
    wF64(m_file, 0.0);                // cn_limit_ext_min
    wF64(m_file, 0.0);                // cn_limit_ext_max
    return currentOffset() - 160;
}

auto Mdf4Writer::writeTxBlock(const std::string& text) -> uint64_t
{
    const uint64_t offset = currentOffset();
    wBlockHdr(m_file, "##TX", txBlockSize(text), 0);
    wTextPadded(m_file, text);
    return offset;
}

auto Mdf4Writer::writeDtBlockHeader() -> uint64_t
{
    const uint64_t offset = currentOffset();
    wBlockHdr(m_file, "##DT", 24, 0);  // block_len patched on close()
    m_dtLenPos = static_cast<std::streampos>(offset + 8);
    return offset;
}

void Mdf4Writer::writeDbcHeader(const std::vector<MessageInfo>& schema, const uint64_t startNs)
{
    const FileLayout layout = computeDbcLayout(schema);

    writeIdBlock();
    writeHdBlock(layout.dgBlock, layout.fhBlock, startNs);
    writeFhBlock(startNs);

    // rec_id_size=1: each record is prefixed with a 1-byte group ID
    const uint64_t firstCgOffset = layout.msgs.empty() ? 0 : layout.msgs.front().cgBlock;
    writeDgBlock(firstCgOffset, layout.dtBlock, 1);

    for (size_t mi = 0; mi < schema.size(); ++mi)
    {
        const auto& [msgId, name, signalList] = schema[mi];
        const MsgLayout& ml = layout.msgs[mi];
        const uint64_t nextCg = (mi + 1 < layout.msgs.size()) ? layout.msgs[mi + 1].cgBlock : 0;

        writeTxBlock(name);  // acquisition name referenced by cg_tx_acqname

        // TX + CN blocks written forward
        const size_t numChannels = ml.channels.size();
        for (size_t ci = 0; ci < numChannels; ++ci)
        {
            const ChanLayout& ch = ml.channels[ci];
            const uint64_t nextCn = (ci + 1 < numChannels) ? ml.channels[ci + 1].cnBlock : 0;

            if (ci == 0)
            {
                writeTxBlock("t");
                writeTxBlock("s");
                writeCnBlock(nextCn, ch.txName, ch.txUnit,
                             /*channelType=*/2, /*syncType=*/1, /*dataType=*/4,
                             /*byteOffset=*/0, /*bitCount=*/64);
            } else
            {
                const auto& sig = signalList[ci - 1];
                writeTxBlock(sig.name);
                if (ch.txUnit != 0) writeTxBlock(sig.unit);
                const uint32_t byteOff = static_cast<uint32_t>(8 + (ci - 1) * 8);
                writeCnBlock(nextCn, ch.txName, ch.txUnit,
                             /*channelType=*/0, /*syncType=*/0, /*dataType=*/4, byteOff,
                             /*bitCount=*/64);
            }
        }

        // cg_data_bytes: 8 (timestamp) + N × 8 (signals)
        const uint32_t dataBytes = static_cast<uint32_t>(8 + signalList.size() * 8);
        writeCgBlock(nextCg, ml.channels.front().cnBlock, ml.txName, static_cast<uint64_t>(mi),
                     dataBytes);

        m_recordCounts.push_back(0);

        MsgMeta meta;
        meta.recordId = static_cast<uint8_t>(mi);
        meta.sigCount = static_cast<uint8_t>(signalList.size());
        meta.recBytes = dataBytes;
        for (const auto& sig : signalList) meta.sigOrder.push_back(sig.name);
        m_msgMeta[msgId] = meta;
    }

    writeDtBlockHeader();
}

void Mdf4Writer::writeRawHeader(const uint64_t startNs)
{
    const auto [fhBlock, dgBlock, msgs, dtBlock] = computeRawLayout();

    writeIdBlock();
    writeHdBlock(dgBlock, fhBlock, startNs);
    writeFhBlock(startNs);
    writeDgBlock(msgs.front().cgBlock, dtBlock, 0);

    const MsgLayout& ml = msgs.front();
    writeTxBlock("CAN");          // cg_tx_acqname
    writeSiBlock(/*busType=*/2);  // 2 = CAN

    // channel[0] = master time "t"
    writeTxBlock("t");
    writeTxBlock("s");
    writeCnBlock(ml.channels[1].cnBlock, ml.channels[0].txName, ml.channels[0].txUnit,
                 /*channelType=*/2, /*syncType=*/1, /*dataType=*/4,
                 /*byteOffset=*/0, /*bitCount=*/64);

    // channel[1] = parent structure channel "CAN_DataFrame"
    writeTxBlock("CAN_DataFrame");
    writeCnBlock(/*nextCn=*/0, ml.channels[1].txName, /*txUnit=*/0,
                 /*channelType=*/0, /*syncType=*/0, /*dataType=*/10,
                 /*byteOffset=*/8, /*bitCount=*/128,
                 /*compositionOffset=*/ml.channels[2].cnBlock);

    // channels[2..7] = composition sub-channels
    struct RawChanDef {
        const char* name;
        uint8_t dtype;
        uint32_t byteOff;
        uint32_t bits;
    };
    constexpr std::array<RawChanDef, 6> subDefs = {{
        {.name = "CAN_DataFrame.BusChannel", .dtype = 0, .byteOff = 8, .bits = 8},
        {.name = "CAN_DataFrame.ID", .dtype = 0, .byteOff = 9, .bits = 32},
        {.name = "CAN_DataFrame.Dir", .dtype = 0, .byteOff = 13, .bits = 8},
        {.name = "CAN_DataFrame.DLC", .dtype = 0, .byteOff = 14, .bits = 8},
        {.name = "CAN_DataFrame.DataLength", .dtype = 0, .byteOff = 15, .bits = 8},
        {.name = "CAN_DataFrame.DataBytes", .dtype = 10, .byteOff = 16, .bits = 64},
    }};

    for (size_t ci = 0; ci < subDefs.size(); ++ci)
    {
        const auto& d = subDefs[ci];
        const size_t chIdx = ci + 2;
        const uint64_t nextCn = (ci + 1 < subDefs.size()) ? ml.channels[chIdx + 1].cnBlock : 0;
        writeTxBlock(d.name);
        writeCnBlock(nextCn, ml.channels[chIdx].txName, /*txUnit=*/0,
                     /*channelType=*/0, /*syncType=*/0, d.dtype, d.byteOff, d.bits);
    }

    writeCgBlock(0, ml.channels[0].cnBlock, ml.txName, 0, 24,
                 /*cgFlags=*/0x0006, ml.siBlock, /*pathSeparator=*/'.');
    m_recordCounts.push_back(0);

    writeDtBlockHeader();
}

Mdf4Writer::Mdf4Writer(const std::string& path, uint64_t, const std::vector<MessageInfo>& schema)
    : m_isDbc(true)
{
    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file.is_open()) throw std::runtime_error("Mdf4Writer: cannot open " + path);

    m_buffer.reserve(Constants::BUFFER_CAPACITY);
    m_startNs = nowNs();
    writeDbcHeader(schema, m_startNs);
    m_file.flush();
}

Mdf4Writer::Mdf4Writer(const std::string& path, uint64_t) : m_isDbc(false)
{
    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file.is_open()) throw std::runtime_error("Mdf4Writer: cannot open " + path);

    m_buffer.reserve(Constants::BUFFER_CAPACITY);
    m_startNs = nowNs();
    writeRawHeader(m_startNs);
    m_file.flush();
}

Mdf4Writer::~Mdf4Writer()
{
    close();
}

void Mdf4Writer::write(const Core::DbcCanMessage& msg)
{
    const auto it = m_msgMeta.find(msg.messageId);
    if (it == m_msgMeta.end()) return;

    const MsgMeta& meta = it->second;

    // Record: [u8 rec_id][f64 t_s][f64 sig0]..[f64 sigN-1]
    const size_t recSize = 1 + 8 + static_cast<size_t>(meta.sigCount) * 8;
    const size_t prevSize = m_buffer.size();
    m_buffer.resize(prevSize + recSize, 0);
    uint8_t* p = m_buffer.data() + prevSize;

    *p++ = meta.recordId;

    const double relTimeSec = static_cast<double>(nowNs() - m_startNs) / 1e9;
    std::memcpy(p, &relTimeSec, 8);
    p += 8;

    // Write values in schema order to match the CN channel byte offsets
    for (const auto& name : meta.sigOrder)
    {
        double value = 0.0;
        for (const auto& sig : msg.signalValues)
        {
            if (sig.name == name)
            {
                value = sig.value;
                break;
            }
        }
        std::memcpy(p, &value, 8);
        p += 8;
    }

    m_recordCounts[meta.recordId]++;
    m_dtBytesWritten += recSize;

    if (m_buffer.size() >= Constants::BUFFER_CAPACITY) flush();
}

void Mdf4Writer::write(const Core::RawCanMessage& msg)
{
    if (m_isDbc) return;

    // ASAM CAN_DataFrame record: [f64 t_s][u8 BusChannel][u32 ID][u8 Dir][u8 DLC][u8
    // DataLength][u8[8] DataBytes]
    constexpr size_t recSize = 24;
    const size_t prevSize = m_buffer.size();
    m_buffer.resize(prevSize + recSize, 0);
    uint8_t* p = m_buffer.data() + prevSize;

    const double relTimeSec = static_cast<double>(nowNs() - m_startNs) / 1e9;
    std::memcpy(p, &relTimeSec, 8);
    p += 8;

    *p++ = 1;  // BusChannel

    const uint32_t id = msg.messageId;
    std::memcpy(p, &id, 4);
    p += 4;

    *p++ = 0;        // Dir: 0 = Rx
    *p++ = msg.dlc;  // DLC
    *p++ = msg.dlc;  // DataLength

    const size_t dataLen = std::min(static_cast<size_t>(msg.dlc), size_t{8});
    std::memcpy(p, msg.data.data(), dataLen);

    m_recordCounts[0]++;
    m_dtBytesWritten += recSize;

    if (m_buffer.size() >= Constants::BUFFER_CAPACITY) flush();
}

void Mdf4Writer::flush()
{
    if (m_buffer.empty()) return;
    wBytes(m_file, m_buffer.data(), m_buffer.size());
    m_buffer.clear();
}

void Mdf4Writer::close()
{
    if (m_closed) return;
    m_closed = true;

    flush();

    // Patch DTBLOCK.block_len = 24 (header) + data bytes written
    patchU64(m_dtLenPos, 24 + m_dtBytesWritten);

    for (size_t i = 0; i < m_cgCyclePos.size(); ++i) patchU64(m_cgCyclePos[i], m_recordCounts[i]);

    m_file.flush();
    m_file.close();
}

auto Mdf4Writer::isOpen() const noexcept -> bool
{
    return m_file.is_open() && !m_closed;
}

auto Mdf4Writer::currentOffset() -> uint64_t
{
    return static_cast<uint64_t>(m_file.tellp());
}

void Mdf4Writer::patchU64(const std::streampos pos, const uint64_t value)
{
    const std::streampos saved = m_file.tellp();
    m_file.seekp(pos);
    wU64(m_file, value);
    m_file.seekp(saved);
}

}  // namespace CanStream