//
// Created by flori on 05.01.2026.
//

#ifndef CANBUSMANAGER_DBC_PARSER_HPP
#define CANBUSMANAGER_DBC_PARSER_HPP
#include <memory>
#include <mutex>
#include <regex>
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
    auto parseDbc() -> std::unique_ptr<Core::DbcConfig>;

   private:
    /**
     * @brief tries to parse the start of the provided file to a signal (SG_:)
     * @return The parsed signal
     */
    auto parseSignal() -> Core::DbcSignalDescription;
    /**
     * @brief tries to parse the start of the provided file to a message (BO_:)
     * @return The parsed message
     */
    auto parseMessage() -> Core::DbcMessageDescription;
    /**
     * @brief tries to parse the start of the provided file to a value description
     * @return The parsed value description
     */
    auto parseValueDescription() -> Core::DbcValueDescription;
    /**
     * @brief tries to parse the start of the provided file to a signal value description (VAL_:)
     * @return The parsed signal value description
     * was parsed
     */
    auto parseSignalValue() -> Core::DbcSignalValueDescription;
    /**
     * @brief tries to parse the start of the provided file to a list of nodes (BU_:)
     * @return The parsed list of nodes
     */
    auto parseNodes() -> std::list<std::string>;
    /**
     * @brief tries to parse the start of the provided file to a comment (CM_:)
     * @return The parsed comment
     */
    auto parseComment() -> std::string;
    /**
     * @brief tries to parse the start of the provided file to a version (Version:)
     * @return The parsed version
     */
    auto parseVersion() -> std::string;
    /**
     * @brief Parses the new symbols field (NS_:)
     */
    void parseNewSymbols();
    /**
     * @brief Parses a value table (VAL_TAB_)
     * @return A bool indicating if a value table has been parsed
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
    /**
     * @brief parses environment variable data (ENVVAR_DATA_)
     */
    void parseEnvironmentVariableData();
    /**
     * @brief Parses environment variable value definitions (VAL_)
     */
    void parseEnvironmentVariableValueDefinitions();
    /**
     * @brief Parses a signal type (SGTYPE_)
     * @return A bool indicating if a signal type has been parsed
     */
    auto parseSignalType() -> bool;
    /**
     * @brief Parses a signal type reference (SGTYPE_)
     * @return A bool indicating if a signal type reference has been parsed
     */
    auto parseSignalTypeReference() -> bool;
    /**
     * @brief Parses a signal group  (SIG_GROUP_)
     */
    void parseSignalGroup();
    /**
     * @brief Parses an attribute definition (BA_DEF_)
     * @return
     */
    auto parseAttributeDefinition() -> bool;
    /**
     * @brief Parses an attribute value (BA_)
     * @returnA bool indicating if an attribute value has been parsed
     */
    auto parseAttributeValue() -> bool;
    /**
     * @brief Parses an attribute default value (BA_DEF_DEF_)
     * @return A bool indicating if an attribute default value has been parsed
     */
    auto parseAttributeDefault() -> bool;
    /**
     * @brief Parses a signal extended value type list (SIG_VALTYPE_)
     */
    void parseSignalExtendedValueTypeList();
    /**
     * @brief Parses the bit timing (BS_:)
     */
    void parseBitTiming();
    /**
     * @brief Removes all spaces at the front of the current @ref CanHandler::file
     */
    void eraseSpaces();
    /**
     * @brief Parses a CIdentifier (testIdentifier dbcFileRest)
     * @return A string containing the CIdentifier (testIdentifier)
     */
    auto parseCIdentifier() -> std::string;
    /**
     * @brief Parses a String ("exampleString" dbcFileRest)
     * @return A string containing the string (exampleString)
     */
    auto parseString() -> std::string;
    /**
     * @brief Parses an unsigned int
     * @return The unsigned int
     */
    auto parseUInt() -> uint;
    /**
     * Parses a signed int
     * @return The signed int
     */
    auto parseInt() -> int;
    /**
     * Parses a double
     * @return The double
     */
    auto parseDouble() -> double;
    /**
     * Truncates the current @ref file to the next semicolon
     * @return A bool indicating there was a semicolon to truncate to
     */
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
    /**
     * @brief Mutex, that protects the file from being overwritten in the parsing process
     */
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

    const std::regex FILE_STARTS_WITH_C_IDENTIFIER_REGEX{"([A-Z]|[a-z]|_)(\\w)+( |:).*"};
    const std::regex FILE_STARTS_WITH_STRING_REGEX{"\".*\".*"};
    const std::regex DOUBLE_REGEX{R"((\+|-)?\d+(\.\d+)?)"};
    const std::regex INT_REGEX{"(\\+|-)?\\d+"};
    const std::regex UINT_REGEX{"\\d+"};
};
}  // namespace CanHandler
#endif  // CANBUSMANAGER_DBC_PARSER_HPP
