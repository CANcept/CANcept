//
// Created by flori on 29.12.2025.
//

#pragma once
#include <array>
#include <chrono>
#include <list>
#include <string>

namespace Core {
struct RawCanMessage {
    std::chrono::milliseconds receiveTime;
    std::array<char, 8> data;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
    uint8_t dlc{8};      // Data Length Code (0-8 bytes)
};
struct DbcCanSignal {
    std::string name;
    double value;
};
struct DbcCanMessage {
    std::chrono::milliseconds receiveTime;
    std::list<DbcCanSignal> signalValues;
    uint16_t messageId;  // CAN ID: 11-bit standard (0x000-0x7FF)
};
}  // namespace Core

