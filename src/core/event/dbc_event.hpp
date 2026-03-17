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
