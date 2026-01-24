//
// Created by flori on 05.01.2026.
//

#ifndef CANBUSMANAGER_DBC_PARSER_HPP
#define CANBUSMANAGER_DBC_PARSER_HPP
#include <cstdint>
#include <mutex>
#include <set>
#include <string>

#include "core/dto/dbc_dto.hpp"
namespace CanHandler {
class DbcParser
{
   public:
    /**
     * @brief Provides a new file for parsing to a DBC config
     * @param newFile The file to parse.
     */
    void provideNewFile(const std::string &newFile);
    /**
     * @brief Tries to parse the provided file to a DBC object
     * @return The parsed dbc config
     */
    auto parseDbc() -> Core::DbcConfig *;

   private:
    /**
     * @brief tries to parse the start of the provided file to a signal (SG_:)
     * @return The parsed signal or a null pointer if no signal was parsed
     */
    auto parseSignal() -> Core::DbcSignalDescription;
    /**
     * @brief tries to parse the start of the provided file to a message (BO_:)
     * @return The parsed message or a null pointer if no message was parsed
     */
    auto parseMessage() -> Core::DbcMessageDescription;
    /**
     * @brief tries to parse the start of the provided file to a value description
     * @return The parsed value description or a null pointer if no value description was parsed
     */
    auto parseValueDescription() -> Core::DbcValueDescription;
    /**
     * @brief tries to parse the start of the provided file to a signal value description (VAL_:)
     * @return The parsed signal value description or a null pointer if no signal value description
     * was parsed
     */
    auto parseSignalValue() -> Core::DbcSignalValueDescription;
    /**
     * @brief tries to parse the start of the provided file to a list of nodes (BU_:)
     * @return The parsed list of nodes or a null pointer if no list of nodes was parsed
     */
    auto parseNodes() -> std::list<std::string>;
    /**
     * @brief tries to parse the start of the provided file to a comment (CM_:)
     * @return The parsed comment or a null pointer if no comment was parsed
     */
    auto parseComment() -> std::string;
    /**
     * @brief tries to parse the start of the provided file to a version (Version:)
     * @return The parsed version or a null pointer if no version was parsed
     */
    auto parseVersion() -> std::string;
    /**
     * @brief Parses the new symbols field (NS_:)
     */
    void parseNewSymbols();
    /**
     * @brief Parses a value table (VAL_TAB_)
     * @return A bool indicating if a value tabel has been parsed
     */
    auto parseValueTable() -> bool;
    /**
     * @brief Parses a message transmitter field (BO_TX_BU_)
     */
    auto parseMessageTransmitter() -> bool;
    /**
     * @brief Parses a environment variable (EV_)
     */
    auto parseEnvironmentVariable() -> bool;
    void parseEnvironmentVariableData();
    void parseEnvironmentVariableValueDefinitions();
    auto parseSignalType() -> bool;
    auto parseSignalTypeReference() -> bool;
    void parseSignalGroup();
    auto parseAttributeDefinition() -> bool;
    auto parseAttributeValue() -> bool;
    auto parseAttributeDefault() -> bool;
    void parseSignalExtendedValueTypeList();
    void parseBitTiming();
    void eraseSpaces();
    auto parseCIdentifier() -> std::string;
    auto parseString() -> std::string;
    auto parseUInt() -> uint;
    auto parseInt() -> int;
    auto parseDouble() -> double;
    auto truncateToNextSemicolon() -> bool;
    /**
     * @brief The current file to parse, truncated to the point of the current parse
     */
    std::string file = "";
    /**
     * @brief Flag indicating if the last parsed object was valid
     */
    bool parsingValid = false;
    /**
     * @brief Flag indicating if the last called function parsed an object
     */
    bool parsedObject = false;
    std::mutex fileMutex;
    const std::pmr::set<std::string> symbols{"CM_",
                                             "BA_DEF_",
                                             "BA_",
                                             "VAL_",
                                             "CAT_DEF_",
                                             "CAT_",
                                             "FILTER",
                                             "BA_DEF_DEF_",
                                             "EV_DATA_",
                                             "ENVVAR_DATA_",
                                             "SGTYPE_",
                                             "SGTYPE_VAL_",
                                             "BA_DEF_SGTYPE_",
                                             "BA_SGTYPE_",
                                             "SIG_TYPE_REF_",
                                             "VAL_TABLE_",
                                             "SIG_GROUP_",
                                             "SIG_VALTYPE_",
                                             "SIGTYPE_VALTYPE_",
                                             "BO_TX_BU_",
                                             "BA_DEF_REL_",
                                             "BA_REL_",
                                             "BA_DEF_DEF_REL_",
                                             "BU_SG_REL_",
                                             "BU_EV_REL_",
                                             "BU_BO_REL_",
                                             "BO_"};
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_DBC_PARSER_HPP
