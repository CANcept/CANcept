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

#include <chrono>
#include <cstring>
#include <stdexcept>

namespace CanStream {

// ─── File write primitives ────────────────────────────────────────────────────

static void wBytes(std::ofstream& f, const void* data, size_t n)
{
    f.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(n));
}
static void wU8(std::ofstream& f, uint8_t v)
{
    wBytes(f, &v, 1);
}
static void wU16(std::ofstream& f, uint16_t v)
{
    wBytes(f, &v, 2);
}
static void wU32(std::ofstream& f, uint32_t v)
{
    wBytes(f, &v, 4);
}
static void wU64(std::ofstream& f, uint64_t v)
{
    wBytes(f, &v, 8);
}
static void wI16(std::ofstream& f, int16_t v)
{
    wBytes(f, &v, 2);
}
static void wF64(std::ofstream& f, double v)
{
    wBytes(f, &v, 8);
}

/** Writes text into a fixed-width field, space-padded (for MDF identifier strings). */
static void wIdent(std::ofstream& f, const char* text, size_t width)
{
    size_t len = std::strlen(text);
    size_t written = std::min(len, width);
    f.write(text, static_cast<std::streamsize>(written));
    for (size_t i = written; i < width; ++i) f.put(' ');
}

/** Writes null-terminated text padded to 8-byte alignment. */
static void wTextPadded(std::ofstream& f, const std::string& text)
{
    f.write(text.data(), static_cast<std::streamsize>(text.size()));
    f.put('\0');
    size_t total = text.size() + 1;
    size_t pad = (8 - (total % 8)) % 8;
    for (size_t i = 0; i < pad; ++i) f.put('\0');
}

/** Writes the 24-byte standard block header (id, reserved, length, link_count). */
static void wBlockHdr(std::ofstream& f, const char* id, uint64_t length, uint64_t linkCount)
{
    f.write(id, 4);
    wU32(f, 0);  // reserved
    wU64(f, length);
    wU64(f, linkCount);
}

// ─── Layout pre-computation ───────────────────────────────────────────────────
// All block offsets are resolved before any writing so every link can be filled
// correctly in one forward pass.

static uint64_t txBlockSize(const std::string& text)
{
    size_t payload = text.size() + 1;            // null-terminated
    size_t padded = (payload + 7) & ~size_t(7);  // align to 8
    return 24 + padded;
}

struct ChanLayout {
    uint64_t txName;
    uint64_t txUnit;  // 0 = no unit TX block
    uint64_t cnBlock;
};

struct MsgLayout {
    uint64_t txName;
    std::vector<ChanLayout> channels;  // [0] = master time, [1..] = signals
    uint64_t cgBlock;
};

struct FileLayout {
    // IDBLOCK is always at 0 (64 bytes)
    // HDBLOCK is always at 64 (104 bytes)
    uint64_t fhBlock;
    uint64_t dgBlock;
    std::vector<MsgLayout> msgs;
    uint64_t dtBlock;
};

static FileLayout computeDbcLayout(const std::vector<MessageInfo>& schema)
{
    FileLayout layout;
    layout.fhBlock = 64 + 104;             // after IDBLOCK + HDBLOCK
    layout.dgBlock = layout.fhBlock + 56;  // after FHBLOCK

    uint64_t pos = layout.dgBlock + 64;  // after DGBLOCK

    for (const auto& msg : schema)
    {
        MsgLayout ml;
        ml.txName = pos;
        pos += txBlockSize(msg.name);

        // Master timestamp channel ("t")
        ChanLayout master;
        master.txName = pos;
        pos += txBlockSize("t");
        master.txUnit = pos;
        pos += txBlockSize("s");
        master.cnBlock = pos;
        pos += 160;
        ml.channels.push_back(master);

        for (const auto& sig : msg.signalList)
        {
            ChanLayout ch;
            ch.txName = pos;
            pos += txBlockSize(sig.name);
            ch.txUnit = sig.unit.empty() ? 0 : pos;
            if (!sig.unit.empty()) pos += txBlockSize(sig.unit);
            ch.cnBlock = pos;
            pos += 160;
            ml.channels.push_back(ch);
        }

        ml.cgBlock = pos;
        pos += 104;
        layout.msgs.push_back(std::move(ml));
    }

    layout.dtBlock = pos;
    return layout;
}

static FileLayout computeRawLayout()
{
    FileLayout layout;
    layout.fhBlock = 64 + 104;
    layout.dgBlock = layout.fhBlock + 56;

    uint64_t pos = layout.dgBlock + 64;

    MsgLayout ml;
    ml.txName = pos;
    pos += txBlockSize("RAW");

    for (const auto& name : {"t", "msg_id", "dlc", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"})
    {
        ChanLayout ch;
        ch.txName = pos;
        pos += txBlockSize(name);
        if (std::string(name) == "t")
        {
            ch.txUnit = pos;
            pos += txBlockSize("s");
        } else
        {
            ch.txUnit = 0;
        }
        ch.cnBlock = pos;
        pos += 160;
        ml.channels.push_back(ch);
    }

    ml.cgBlock = pos;
    pos += 104;
    layout.dtBlock = pos;
    layout.msgs.push_back(std::move(ml));
    return layout;
}

// ─── Block writers ────────────────────────────────────────────────────────────

void Mdf4Writer::writeIdBlock()
{
    // Fixed 64-byte identification block — no standard block header
    wIdent(m_file, "MDF     ", 8);
    wIdent(m_file, "4.10    ", 8);
    wIdent(m_file, "CanBusMgr", 8);               // program identifier
    wU32(m_file, 0);                              // reserved
    wU16(m_file, 410);                            // version = 4.10
    wU16(m_file, 0);                              // reserved
    wU16(m_file, 0);                              // unfinished flags
    wU16(m_file, 0);                              // custom unfinished flags
    for (int i = 0; i < 28; ++i) wU8(m_file, 0);  // reserved to reach 64 bytes
    // 8+8+8+4+2+2+2+2+28 = 64 ✓
}

uint64_t Mdf4Writer::writeHdBlock(uint64_t dgOffset, uint64_t fhOffset, uint64_t startNs)
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
    wI16(m_file, 0);         // tz offset (UTC)
    wI16(m_file, 0);         // DST offset
    wU8(m_file, 0);          // time flags
    wU8(m_file, 0);          // time class
    wU8(m_file, 0);          // hd_flags
    wU8(m_file, 0);          // reserved
    wF64(m_file, 0.0);       // hd_start_angle
    wF64(m_file, 0.0);       // hd_start_distance
    return 64;
}

uint64_t Mdf4Writer::writeFhBlock(uint64_t startNs)
{
    // 24 (header) + 2×8 (links) + 16 (data) = 56 bytes
    wBlockHdr(m_file, "##FH", 56, 2);
    wU64(m_file, 0);        // fh_fh_next
    wU64(m_file, 0);        // fh_md_comment
    wU64(m_file, startNs);  // fh_time_ns
    wI16(m_file, 0);        // tz offset
    wI16(m_file, 0);        // DST offset
    wU8(m_file, 0);         // time flags
    wU8(m_file, 0);         // reserved
    wU16(m_file, 0);        // reserved
    return currentOffset() - 56;
}

uint64_t Mdf4Writer::writeDgBlock(uint64_t firstCgOffset, uint64_t dtOffset, uint8_t recIdSize)
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

uint64_t Mdf4Writer::writeCgBlock(uint64_t nextCgOffset, uint64_t firstCnOffset, uint64_t recordId,
                                  uint32_t dataBytes)
{
    // 24 (header) + 6×8 (links) + 32 (data) = 104 bytes
    wBlockHdr(m_file, "##CG", 104, 6);
    wU64(m_file, nextCgOffset);   // cg_cg_next
    wU64(m_file, firstCnOffset);  // cg_cn_first
    wU64(m_file, 0);              // cg_tx_acqname
    wU64(m_file, 0);              // cg_si_acqsource
    wU64(m_file, 0);              // cg_sr_first
    wU64(m_file, 0);              // cg_md_comment
    wU64(m_file, recordId);       // cg_record_id
    // cg_cycle_count — save position to patch on close
    m_cgCyclePos.push_back(static_cast<std::streampos>(currentOffset()));
    wU64(m_file, 0);          // cg_cycle_count (patched on close)
    wU16(m_file, 0);          // cg_flags
    wU16(m_file, 0);          // cg_path_separator
    wU32(m_file, 0);          // reserved
    wU32(m_file, dataBytes);  // cg_data_bytes
    wU32(m_file, 0);          // cg_inval_bytes
    return currentOffset() - 104;
}

uint64_t Mdf4Writer::writeCnBlock(uint64_t nextCnOffset, uint64_t txNameOffset,
                                  uint64_t txUnitOffset, uint8_t channelType, uint8_t syncType,
                                  uint8_t dataType, uint32_t byteOffset, uint32_t bitCount)
{
    // 24 (header) + 8×8 (links) + 72 (data) = 160 bytes
    wBlockHdr(m_file, "##CN", 160, 8);
    wU64(m_file, nextCnOffset);  // cn_cn_next
    wU64(m_file, 0);             // cn_composition
    wU64(m_file, txNameOffset);  // cn_tx_name
    wU64(m_file, 0);             // cn_si_source
    wU64(m_file, 0);             // cn_cc_conversion (0 = identity)
    wU64(m_file, 0);             // cn_data
    wU64(m_file, txUnitOffset);  // cn_md_unit
    wU64(m_file, 0);             // cn_md_comment
    // data (72 bytes)
    wU8(m_file, channelType);  // cn_channel_type
    wU8(m_file, syncType);     // cn_sync_type
    wU8(m_file, dataType);     // cn_data_type
    wU8(m_file, 0);            // cn_bit_offset
    wU32(m_file, byteOffset);  // cn_byte_offset
    wU32(m_file, bitCount);    // cn_bit_count
    wU32(m_file, 0);           // cn_flags
    wU32(m_file, 0);           // cn_inval_bit_pos
    wU8(m_file, 0xFF);         // cn_precision (0xFF = no restriction)
    wU8(m_file, 0);            // reserved
    wU16(m_file, 0);           // cn_val_range_n (0 = not set)
    wF64(m_file, 0.0);         // cn_val_range_min
    wF64(m_file, 0.0);         // cn_val_range_max
    wF64(m_file, 0.0);         // cn_limit_min
    wF64(m_file, 0.0);         // cn_limit_max
    wF64(m_file, 0.0);         // cn_limit_ext_min
    wF64(m_file, 0.0);         // cn_limit_ext_max
    return currentOffset() - 160;
}

uint64_t Mdf4Writer::writeTxBlock(const std::string& text)
{
    uint64_t offset = currentOffset();
    uint64_t len = txBlockSize(text);  // 24 + padded text
    wBlockHdr(m_file, "##TX", len, 0);
    wTextPadded(m_file, text);
    return offset;
}

uint64_t Mdf4Writer::writeDtBlockHeader()
{
    uint64_t offset = currentOffset();
    wBlockHdr(m_file, "##DT", 24, 0);  // length patched on close
    // Save position of block_len field (at offset 8 within the block header)
    m_dtLenPos = static_cast<std::streampos>(offset + 8);
    return offset;
}

// ─── Header orchestration ─────────────────────────────────────────────────────

void Mdf4Writer::writeDbcHeader(const std::vector<MessageInfo>& schema, uint64_t startNs)
{
    const FileLayout layout = computeDbcLayout(schema);

    writeIdBlock();                                         // offset 0
    writeHdBlock(layout.dgBlock, layout.fhBlock, startNs);  // offset 64
    writeFhBlock(startNs);

    // DGBLOCK: rec_id_size = 1 (one byte per record to identify the channel group)
    const uint64_t firstCgOffset = layout.msgs.empty() ? 0 : layout.msgs.front().cgBlock;
    writeDgBlock(firstCgOffset, layout.dtBlock, 1);

    // Write TX/CN blocks and CGBLOCKs for each selected message
    for (size_t mi = 0; mi < schema.size(); ++mi)
    {
        const MessageInfo& msg = schema[mi];
        const MsgLayout& ml = layout.msgs[mi];
        const uint64_t nextCg = (mi + 1 < layout.msgs.size()) ? layout.msgs[mi + 1].cgBlock : 0;

        // TX block for message (acquisition) name
        writeTxBlock(msg.name);

        // TX + CN blocks in forward order — layout was precomputed so cn_cn_next
        // offsets are already known for every channel regardless of write order.
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
                             /*channelType=*/2, /*syncType=*/1, /*dataType=*/7,
                             /*byteOffset=*/0, /*bitCount=*/64);
            } else
            {
                const auto& sig = msg.signalList[ci - 1];
                writeTxBlock(sig.name);
                if (ch.txUnit != 0) writeTxBlock(sig.unit);
                const uint32_t byteOff = static_cast<uint32_t>(8 + (ci - 1) * 8);
                writeCnBlock(nextCn, ch.txName, ch.txUnit,
                             /*channelType=*/0, /*syncType=*/0, /*dataType=*/7, byteOff,
                             /*bitCount=*/64);
            }
        }

        // cg_data_bytes: 8 (timestamp) + N × 8 (signals)
        const uint32_t dataBytes = static_cast<uint32_t>(8 + msg.signalList.size() * 8);
        writeCgBlock(nextCg, ml.channels.front().cnBlock, static_cast<uint64_t>(mi), dataBytes);

        m_recordCounts.push_back(0);

        MsgMeta meta;
        meta.recordId = static_cast<uint8_t>(mi);
        meta.sigCount = static_cast<uint8_t>(msg.signalList.size());
        meta.recBytes = dataBytes;
        for (const auto& sig : msg.signalList) meta.sigOrder.push_back(sig.name);
        m_msgMeta[msg.msgId] = meta;
    }

    writeDtBlockHeader();
}

void Mdf4Writer::writeRawHeader(uint64_t startNs)
{
    const FileLayout layout = computeRawLayout();

    writeIdBlock();
    writeHdBlock(layout.dgBlock, layout.fhBlock, startNs);
    writeFhBlock(startNs);
    writeDgBlock(layout.msgs.front().cgBlock, layout.dtBlock, 0);

    const MsgLayout& ml = layout.msgs.front();
    writeTxBlock("RAW");

    // Channel definitions for RAW records:
    // [0] t       uint64  offset  0  bits 64  MASTER TIME
    // [1] msg_id  uint16  offset  8  bits 16
    // [2] dlc     uint8   offset 10  bits  8
    // [3..10] d0..d7 uint8 offset 11..18
    struct RawChanDef {
        const char* name;
        uint8_t dtype;
        uint32_t byteOff;
        uint32_t bits;
    };
    const RawChanDef defs[] = {
        {"t", 7, 0, 64},  {"msg_id", 0, 8, 16}, {"dlc", 0, 10, 8}, {"d0", 0, 11, 8},
        {"d1", 0, 12, 8}, {"d2", 0, 13, 8},     {"d3", 0, 14, 8},  {"d4", 0, 15, 8},
        {"d5", 0, 16, 8}, {"d6", 0, 17, 8},     {"d7", 0, 18, 8},
    };

    constexpr size_t numCh = std::size(defs);
    for (size_t ci = 0; ci < numCh; ++ci)
    {
        const auto& d = defs[ci];
        const uint64_t nextCn = (ci + 1 < numCh) ? ml.channels[ci + 1].cnBlock : 0;
        writeTxBlock(d.name);
        if (ci == 0)
        {
            writeTxBlock("s");
        }
        const uint8_t chType = (ci == 0) ? 2 : 0;
        const uint8_t synType = (ci == 0) ? 1 : 0;
        writeCnBlock(nextCn, ml.channels[ci].txName, 0, chType, synType, d.dtype, d.byteOff,
                     d.bits);
    }

    // cg_data_bytes = 8+2+1+8 = 19, rec_id_size = 0 so no prefix
    writeCgBlock(0, ml.channels.front().cnBlock, 0, 19);
    m_recordCounts.push_back(0);

    writeDtBlockHeader();
}

// ─── Constructors / destructor ────────────────────────────────────────────────

static uint64_t nowNs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
}

Mdf4Writer::Mdf4Writer(const std::string& path, uint64_t /*sessionId*/,
                       const std::vector<MessageInfo>& schema)
    : m_isDbc(true)
{
    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file.is_open()) throw std::runtime_error("Mdf4Writer: cannot open " + path);

    m_buffer.reserve(k_bufferCapacity);
    m_startNs = nowNs();
    writeDbcHeader(schema, nowNs());
    m_file.flush();
}

Mdf4Writer::Mdf4Writer(const std::string& path, uint64_t /*sessionId*/) : m_isDbc(false)
{
    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file.is_open()) throw std::runtime_error("Mdf4Writer: cannot open " + path);

    m_buffer.reserve(k_bufferCapacity);
    m_startNs = nowNs();
    writeRawHeader(nowNs());
    m_file.flush();
}

Mdf4Writer::~Mdf4Writer()
{
    close();
}

// ─── Record writing ───────────────────────────────────────────────────────────

void Mdf4Writer::write(const Core::DbcCanMessage& msg)
{
    auto it = m_msgMeta.find(msg.messageId);
    if (it == m_msgMeta.end()) return;

    const MsgMeta& meta = it->second;

    // Record layout: [uint8 rec_id][uint64 ts_ns][double sig0]..[double sigN-1]
    const size_t recSize = 1 + 8 + static_cast<size_t>(meta.sigCount) * 8;
    const size_t prevSize = m_buffer.size();
    m_buffer.resize(prevSize + recSize, 0);
    uint8_t* p = m_buffer.data() + prevSize;

    *p++ = meta.recordId;

    const double relTimeSec = static_cast<double>(nowNs() - m_startNs) / 1e9;
    std::memcpy(p, &relTimeSec, 8);
    p += 8;

    // Write values in schema order so they align with the CN channel definitions
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

    if (m_buffer.size() >= k_bufferCapacity) flush();
}

void Mdf4Writer::write(const Core::RawCanMessage& msg)
{
    if (m_isDbc) return;

    // Record layout (no rec_id prefix, dg_rec_id_size = 0):
    // [uint64 ts_ns][uint16 msg_id][uint8 dlc][uint8 d0..d7]
    constexpr size_t recSize = 19;
    const size_t prevSize = m_buffer.size();
    m_buffer.resize(prevSize + recSize, 0);
    uint8_t* p = m_buffer.data() + prevSize;

    const double relTimeSec = static_cast<double>(nowNs() - m_startNs) / 1e9;
    std::memcpy(p, &relTimeSec, 8);
    p += 8;

    const uint16_t id = msg.messageId;
    std::memcpy(p, &id, 2);
    p += 2;

    *p++ = msg.dlc;

    const size_t dataBytes = std::min(static_cast<size_t>(msg.dlc), size_t{8});
    std::memcpy(p, msg.data.data(), dataBytes);  // array<char,8> → void* is fine for memcpy

    m_recordCounts[0]++;
    m_dtBytesWritten += recSize;

    if (m_buffer.size() >= k_bufferCapacity) flush();
}

// ─── Flush / close ────────────────────────────────────────────────────────────

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

    // Patch DTBLOCK.block_len = 24 (header) + bytes written
    patchU64(m_dtLenPos, 24 + m_dtBytesWritten);

    // Patch each CGBLOCK.cg_cycle_count
    for (size_t i = 0; i < m_cgCyclePos.size(); ++i) patchU64(m_cgCyclePos[i], m_recordCounts[i]);

    m_file.flush();
    m_file.close();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

bool Mdf4Writer::isOpen() const noexcept
{
    return m_file.is_open() && !m_closed;
}

uint64_t Mdf4Writer::currentOffset()
{
    return static_cast<uint64_t>(m_file.tellp());
}

void Mdf4Writer::patchU64(std::streampos pos, uint64_t value)
{
    const std::streampos saved = m_file.tellp();
    m_file.seekp(pos);
    wU64(m_file, value);
    m_file.seekp(saved);
}

void Mdf4Writer::appendRecord(const void* data, size_t size)
{
    const auto* bytes = reinterpret_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), bytes, bytes + size);
    m_dtBytesWritten += size;
    if (m_buffer.size() >= k_bufferCapacity) flush();
}

}  // namespace CanStream