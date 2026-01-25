//
// Created by flori on 29.12.2025.
//

#ifndef CANBUSMANAGER_DBC_EVENT_HPP
#define CANBUSMANAGER_DBC_EVENT_HPP

#include "core/dto/dbc_dto.hpp"
#include "event.hpp"
namespace Core {

/**
 * @brief Structure of the event fired when dbc file has successfully been parsed by the CAN
 * Handler.
 */
struct DBCParsedEvent final : Event {
    DbcConfig config;
    std::string filePath;

    DBCParsedEvent(DbcConfig config, std::string filePath)
        : config(std::move(config)), filePath(std::move(filePath)){};
};

/**
 * @brief Structure of the event fired when dbc file parsing failed.
 */
struct DBCParseErrorEvent final : Event {
    std::string errorMessage;
    std::string filePath;

    DBCParseErrorEvent(std::string errorMessage, std::string filePath)
        : errorMessage(std::move(errorMessage)), filePath(std::move(filePath)){};
};

/**
 * @brief Structure of the event fired when a dbc file is requested to be parsed.
 */
struct ParseDBCRequestEvent final : Event {
    std::string filePath;

    explicit ParseDBCRequestEvent(std::string filePath) : filePath(std::move(filePath)){};
};
}  // namespace Core
#endif  // CANBUSMANAGER_DBC_EVENT_HPP
