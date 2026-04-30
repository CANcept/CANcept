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

#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "can_stream/can_stream_schema.hpp"

// Internal helpers for mdf4_writer.cpp — not part of the public API.

namespace CanStream {

static uint64_t nowNs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
}

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

/** Fixed-width field, space-padded to `width` bytes (MDF4 identifier strings). */
static void wIdent(std::ofstream& f, const char* text, size_t width)
{
    size_t len = std::strlen(text);
    size_t written = std::min(len, width);
    f.write(text, static_cast<std::streamsize>(written));
    for (size_t i = written; i < width; ++i) f.put(' ');
}

/** Null-terminated text padded to 8-byte alignment. */
static void wTextPadded(std::ofstream& f, const std::string& text)
{
    f.write(text.data(), static_cast<std::streamsize>(text.size()));
    f.put('\0');
    size_t total = text.size() + 1;
    size_t pad = (8 - (total % 8)) % 8;
    for (size_t i = 0; i < pad; ++i) f.put('\0');
}

/** Standard 24-byte block header: [id:4][reserved:4][length:8][link_count:8]. */
static void wBlockHdr(std::ofstream& f, const char* id, uint64_t length, uint64_t linkCount)
{
    f.write(id, 4);
    wU32(f, 0);
    wU64(f, length);
    wU64(f, linkCount);
}

// All block offsets are resolved before any writing so every forward link can be
// filled correctly in a single pass.

static uint64_t txBlockSize(const std::string& text)
{
    size_t payload = text.size() + 1;
    size_t padded = (payload + 7) & ~size_t(7);
    return 24 + padded;
}

static std::string mdBlockXml(uint16_t msgId)
{
    char hex[8];
    std::snprintf(hex, sizeof(hex), "%X", msgId);
    return std::string("<CANmessage><id>0x") + hex + "</id></CANmessage>";
}

struct ChanLayout {
    uint64_t txName;
    uint64_t txUnit;  // 0 = no unit block
    uint64_t cnBlock;
};

struct MsgLayout {
    uint64_t txName;
    uint64_t mdBlock{0};               // MDBLOCK for CAN message ID metadata (DBC sessions)
    uint64_t siBlock{0};               // 0 = no SI block (DBC sessions)
    std::vector<ChanLayout> channels;  // [0] = master time, [1..] = signals
    uint64_t cgBlock;
};

struct FileLayout {
    uint64_t fhBlock;
    uint64_t dgBlock;
    std::vector<MsgLayout> msgs;
    uint64_t dtBlock;
};

static FileLayout computeDbcLayout(const std::vector<MessageInfo>& schema)
{
    FileLayout layout;
    layout.fhBlock = 64 + 104;             // IDBLOCK + HDBLOCK
    layout.dgBlock = layout.fhBlock + 56;  // + FHBLOCK

    uint64_t pos = layout.dgBlock + 64;  // + DGBLOCK

    for (const auto& msg : schema)
    {
        MsgLayout ml;
        ml.txName = pos;
        pos += txBlockSize(msg.name);
        ml.mdBlock = pos;
        pos += txBlockSize(mdBlockXml(msg.msgId));

        // Master time channel: TX("t") + TX("s" unit) + CNBLOCK
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
    pos += txBlockSize("CAN");  // cg_tx_acqname = "CAN"
    ml.siBlock = pos;
    pos += 56;  // SIBLOCK: 24 (header) + 3×8 (links) + 8 (data)

    // [0]=master "t", [1]=parent "CAN_DataFrame", [2..7]=sub-channels
    constexpr std::array<const char*, 8> chanNames = {
        "t",
        "CAN_DataFrame",
        "CAN_DataFrame.BusChannel",
        "CAN_DataFrame.ID",
        "CAN_DataFrame.Dir",
        "CAN_DataFrame.DLC",
        "CAN_DataFrame.DataLength",
        "CAN_DataFrame.DataBytes",
    };

    for (size_t i = 0; i < chanNames.size(); ++i)
    {
        ChanLayout ch;
        ch.txName = pos;
        pos += txBlockSize(chanNames[i]);
        ch.txUnit = (i == 0) ? pos : 0;
        if (i == 0) pos += txBlockSize("s");
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

}  // namespace CanStream