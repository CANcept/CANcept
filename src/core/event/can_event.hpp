//
// Created by flori on 28.12.2025.
//

#ifndef CANBUSMANAGER_CAN_EVENT_HPP
#define CANBUSMANAGER_CAN_EVENT_HPP
#include <array>
#include <ctime>
#include <string>
#include <unordered_map>

#include "core/dto/can_dto.hpp"
#include "event.hpp"
namespace Core {
/**
 * @brief Structure of the received can event when a can message is received and used in raw form
 */
struct ReceivedCanRawEvent final : public Event {
    RawCanMessage canMessage;

    explicit ReceivedCanRawEvent(const RawCanMessage& canMessage);
};
/**
 * @brief Structure of the received can event when a can message is received and used in dbc decoded
 * form
 */
struct ReceivedCanDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit ReceivedCanDbcEvent(const DbcCanMessage& canMessage);
};
/**
 * @brief Structure of the send can event, when an already encoded message should be sent
 */
struct SendCanMessageRawEvent final : public Event {
    RawCanMessage canMessage;

    explicit SendCanMessageRawEvent(const RawCanMessage& canMessage);
};
/**
 * @brief Structure of the send can event, when a message should be sent based on the current DBC
 * config
 */
struct SendCanMessageDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit SendCanMessageDbcEvent(const DbcCanMessage& canMessage);
};
};  // namespace Core
#endif  // CANBUSMANAGER_CAN_EVENT_HPP
