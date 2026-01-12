//
// Created by flori on 05.01.2026.
//

#ifndef CANBUSMANAGER_DBC_PARSER_HPP
#define CANBUSMANAGER_DBC_PARSER_HPP
#include <string>

#include "core/dto/dbc_dto.hpp"
namespace CanHandler {
class DbcParser
{
   public:
    /**
     * @brief Provides a new file for parsing to a DBC config
     * @param file The file to parse.
     */
    void provideNewFile(std::string file);
    /**
     * @brief Tries to parse the provided file to a DBC object
     * @return The parsed dbc config
     */
    auto parseDbc() -> Core::DbcConfig*;

   private:
    /**
     * @brief tries to parse the start of the provided file to a signal (SG_:)
     * @return The parsed signal or a null pointer if no signal was parsed
     */
    auto parseSignal() -> Core::DbcSignalDescription*;
    /**
     * @brief tries to parse the start of the provided file to a message (BO_:)
     * @return The parsed message or a null pointer if no message was parsed
     */
    auto parseMessage() -> Core::DbcMessageDescription*;
    /**
     * @brief tries to parse the start of the provided file to a value description
     * @return The parsed value description or a null pointer if no value description was parsed
     */
    auto parseValueDescription() -> Core::DbcValueDescription*;
    /**
     * @brief tries to parse the start of the provided file to a signal value description (VAL_:)
     * @return The parsed signal value description or a null pointer if no signal value description
     * was parsed
     */
    auto parseSignalValue() -> Core::DbcSignalValueDescription*;
    /**
     * @brief tries to parse the start of the provided file to a list of nodes (BU_:)
     * @return The parsed list of nodes or a null pointer if no list of nodes was parsed
     */
    auto parseNodes() -> std::list<std::string>*;
    /**
     * @brief tries to parse the start of the provided file to a comment (CM_:)
     * @return The parsed comment or a null pointer if no comment was parsed
     */
    auto parseComment() -> std::string*;
    /**
     * @brief The current file to parse, truncated to the point of the current parse
     */
    std::string file;
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_DBC_PARSER_HPP
