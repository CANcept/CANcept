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
#include <ios>
#include <string>
#include <unordered_map>
#include <vector>

#include "can_stream/can_stream_schema.hpp"
#include "core/dto/can_dto.hpp"

namespace CanStream {

/**
 * @brief Streaming MDF4 writer for decoded CAN data.
 *
 * Writes a minimal but spec-compliant MDF4 file (.mf4).
 * The file header and channel schema are written on construction.
 * Data records are buffered and flushed to disk periodically via flush()
 * or automatically when the internal buffer reaches capacity.
 * close() must be called (or the destructor will call it) to finalize
 * the file — it patches cycle counts and the data block length.
 *
 * Thread safety: write() and flush() must be called from the same thread.
 */
class Mdf4Writer
{
   public:
    /** @brief Opens a DBC-decoded session file. Schema must be non-empty. */
    Mdf4Writer(const std::string& path, uint64_t sessionId, const std::vector<MessageInfo>& schema);

    /** @brief Opens a RAW frame session file. */
    Mdf4Writer(const std::string& path, uint64_t sessionId);

    ~Mdf4Writer();

    Mdf4Writer(const Mdf4Writer&) = delete;
    Mdf4Writer& operator=(const Mdf4Writer&) = delete;

    void write(const Core::DbcCanMessage& msg);
    void write(const Core::RawCanMessage& msg);

    /** @brief Flushes the write buffer to disk. Call periodically (e.g. every 100 ms). */
    void flush();

    /** @brief Finalizes the file. Safe to call multiple times. */
    void close();

    [[nodiscard]] bool isOpen() const noexcept;

   private:
    struct MsgMeta {
        uint8_t recordId;                   // cg_record_id for this channel group
        uint8_t sigCount;                   // number of signal channels
        uint32_t recBytes;                  // cg_data_bytes (record size excluding rec_id prefix)
        std::vector<std::string> sigOrder;  // signal names in schema order for value lookup
    };

    // --- MDF4 block writers (return the file offset of the written block) ---
    void writeIdBlock();
    uint64_t writeHdBlock(uint64_t dgOffset, uint64_t fhOffset, uint64_t startNs);
    uint64_t writeFhBlock(uint64_t startNs);
    uint64_t writeDgBlock(uint64_t firstCgOffset, uint64_t dtOffset, uint8_t recIdSize);
    uint64_t writeCgBlock(uint64_t nextCgOffset, uint64_t firstCnOffset, uint64_t recordId,
                          uint32_t dataBytes);
    uint64_t writeCnBlock(uint64_t nextCnOffset, uint64_t txNameOffset, uint64_t txUnitOffset,
                          uint8_t channelType, uint8_t syncType, uint8_t dataType,
                          uint32_t byteOffset, uint32_t bitCount);
    uint64_t writeTxBlock(const std::string& text);
    uint64_t writeDtBlockHeader();

    // --- Helpers ---
    void writeDbcHeader(const std::vector<MessageInfo>& schema, uint64_t startNs);
    void writeRawHeader(uint64_t startNs);
    void appendRecord(const void* data, size_t size);

    /** @brief Seeks back to pos and writes a uint64 without disturbing the write position. */
    void patchU64(std::streampos pos, uint64_t value);

    [[nodiscard]] uint64_t currentOffset();

    std::ofstream m_file;

    // Write buffer — flushed when full or via flush()
    static constexpr size_t k_bufferCapacity = 4 * 1024 * 1024;
    std::vector<uint8_t> m_buffer;

    // Positions to patch on close()
    std::streampos m_dtLenPos;                 // DTBLOCK.block_len
    std::vector<std::streampos> m_cgCyclePos;  // one per channel group

    std::unordered_map<uint16_t, MsgMeta> m_msgMeta;
    std::vector<uint64_t> m_recordCounts;  // indexed by recordId

    uint64_t m_startNs{0};
    uint64_t m_dtBytesWritten{0};
    bool m_closed{false};
    bool m_isDbc{false};
};

}  // namespace CanStream