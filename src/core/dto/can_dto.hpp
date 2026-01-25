//
// Created by flori on 29.12.2025.
//

#ifndef CANBUSMANAGER_CAN_DTO_HPP
#define CANBUSMANAGER_CAN_DTO_HPP
#include <array>
#include <chrono>
#include <list>
#include <string>

namespace Core {
struct RawCanMessage {
    std::chrono::milliseconds receiveTime;
    std::array<char, 8> data;
    char messageId;
};
struct DbcCanSignal {
    std::string name;
    double value;
};
struct DbcCanMessage {
    std::chrono::milliseconds receiveTime;
    std::list<DbcCanSignal> signalValues;
    char messageId;
};
}  // namespace Core
#endif  // CANBUSMANAGER_CAN_DTO_HPP
