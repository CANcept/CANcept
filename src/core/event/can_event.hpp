#pragma once
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

    explicit ReceivedCanRawEvent(const RawCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the received can event when a can message is received and used in dbc decoded
 * form
 */
struct ReceivedCanDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit ReceivedCanDbcEvent(const DbcCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the send can event, when an already encoded message should be sent
 */
struct SendCanMessageRawEvent final : public Event {
    RawCanMessage canMessage;

    explicit SendCanMessageRawEvent(const RawCanMessage& canMessage) : canMessage(canMessage){};
};
/**
 * @brief Structure of the send can event, when a message should be sent based on the current DBC
 * config
 */
struct SendCanMessageDbcEvent final : public Event {
    DbcCanMessage canMessage;

    explicit SendCanMessageDbcEvent(const DbcCanMessage& canMessage) : canMessage(canMessage){};
};
};  // namespace Core
