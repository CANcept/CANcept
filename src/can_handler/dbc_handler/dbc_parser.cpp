#include "dbc_parser.hpp"

#include <iostream>
#include <regex>

#include "core/macro/console_logging.hpp"
namespace CanHandler {
#define IF_PARSING_INVALID_RETURN(returnValue) \
    if (!parsingValid)                         \
    {                                          \
        parsedObject = false;                  \
        return returnValue;                    \
    }
void DbcParser::provideNewFile(const std::string& newFile)
{
    std::scoped_lock guard(fileMutex);
    file = newFile;
}
auto DbcParser::parseDbc() -> std::unique_ptr<Core::DbcConfig>
{
    std::scoped_lock guard(fileMutex);
    parsingValid = true;
    parsedObject = true;
    const std::string version = parseVersion();
    parseNewSymbols();
    IF_PARSING_INVALID_RETURN({nullptr})
    parseBitTiming();
    IF_PARSING_INVALID_RETURN({nullptr})
    std::list<std::string> nodes = parseNodes();
    IF_PARSING_INVALID_RETURN({nullptr})
    while (parseValueTable())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    std::list<Core::DbcMessageDescription> messageDescriptions;
    while (true)
    {
        const Core::DbcMessageDescription messageDescription = parseMessage();
        if (!parsedObject)
        {
            IF_PARSING_INVALID_RETURN({nullptr})
            break;
        }
        messageDescriptions.push_back(messageDescription);
    }
    while (parseMessageTransmitter())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    while (parseEnvironmentVariable())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    parseEnvironmentVariableData();
    IF_PARSING_INVALID_RETURN({nullptr})
    while (parseSignalType())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    std::list<std::string> comments;
    while (true)
    {
        const std::string comment = parseComment();
        if (!parsedObject)
        {
            IF_PARSING_INVALID_RETURN({nullptr})
            break;
        }
        comments.push_back(comment);
    }
    while (parseAttributeDefinition())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    while (parseAttributeDefault())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    while (parseAttributeValue())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    while (true)
    {
        const std::string comment = parseComment();
        if (!parsedObject)
        {
            IF_PARSING_INVALID_RETURN({nullptr})
            break;
        }
        comments.push_back(comment);
    }
    std::list<Core::DbcSignalValueDescription> valueDescriptions;
    while (true)
    {
        const Core::DbcSignalValueDescription valueDescription = parseSignalValue();
        if (!parsedObject)
        {
            IF_PARSING_INVALID_RETURN({nullptr})
            break;
        }
        if (valueDescription.messageId == static_cast<uint>(-1))
        {
            continue;
        }
        valueDescriptions.push_back(valueDescription);
    }
    while (parseSignalTypeReference())
    {
        IF_PARSING_INVALID_RETURN({nullptr})
    }
    parseSignalGroup();
    IF_PARSING_INVALID_RETURN({nullptr})
    parseSignalExtendedValueTypeList();
    IF_PARSING_INVALID_RETURN({nullptr})
    eraseSpaces();
    if (file != "")
    {
        return {nullptr};
    }
    return std::unique_ptr<Core::DbcConfig>(new Core::DbcConfig(
        nodes, messageDescriptions, valueDescriptions, comments, {.version = version}));
};

auto DbcParser::parseComment() -> std::string
{
    eraseSpaces();
    if (!file.starts_with("CM_"))
    {
        parsedObject = false;
        return "";
    }
    file = file.substr(3);
    eraseSpaces();
    std::string preComment = "";
    if (file.starts_with("BU_"))
    {
        file = file.substr(3);
        preComment = "Node: ";
        preComment += parseCIdentifier() + " ";
    } else if (file.starts_with("BO_"))
    {
        file = file.substr(3);
        preComment = "Message: ";
        preComment += std::to_string(parseUInt()) + " ";
    } else if (file.starts_with("SG_"))
    {
        file = file.substr(3);
        preComment = "Signal: ";
        preComment += std::to_string(parseUInt()) + " ";
        preComment += parseCIdentifier() + " ";
    } else if (file.starts_with("EV_"))
    {
        file = file.substr(3);
        preComment = "Environment variable: ";
        preComment += parseCIdentifier() + " ";
    }
    std::string comment = parseString();
    IF_PARSING_INVALID_RETURN("")
    eraseSpaces();
    if (!file.starts_with(";"))
    {
        parsedObject = false;
        parsingValid = false;
        return "";
    }
    file = file.substr(1);
    parsedObject = true;
    return preComment + comment;
}
auto DbcParser::parseMessage() -> Core::DbcMessageDescription
{
    eraseSpaces();
    if (!file.starts_with("BO_"))
    {
        parsedObject = false;
        return {};
    }
    file = file.substr(3);
    const uint messageId = parseUInt();
    IF_PARSING_INVALID_RETURN(Core::DbcMessageDescription())
    const std::string messageName = parseCIdentifier();
    IF_PARSING_INVALID_RETURN(Core::DbcMessageDescription())
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        parsingValid = false;
        parsedObject = false;
        return {};
    }
    file = file.substr(1);
    const uint messageSize = parseUInt();
    IF_PARSING_INVALID_RETURN(Core::DbcMessageDescription())
    const std::string transmitter = parseCIdentifier();
    IF_PARSING_INVALID_RETURN(Core::DbcMessageDescription())
    Core::DbcMessageDescription messageDescription{.messageId = messageId,
                                                   .messageName = messageName,
                                                   .messageSize = messageSize,
                                                   .transmitterName = transmitter};
    while (true)
    {
        const Core::DbcSignalDescription signalDescription = parseSignal();
        if (!parsedObject)
        {
            IF_PARSING_INVALID_RETURN(Core::DbcMessageDescription())
            break;
        }
        messageDescription.signalDescriptions.push_back(signalDescription);
    }
    parsedObject = true;
    return messageDescription;
}
auto DbcParser::parseNodes() -> std::list<std::string>
{
    eraseSpaces();
    if (!file.starts_with("BU_:"))
    {
        parsingValid = false;
        parsedObject = false;
        return {};
    }
    file = file.substr(4);
    std::list<std::string> nodes{};
    while (true)
    {
        std::string node = parseCIdentifier();
        IF_PARSING_INVALID_RETURN(std::list<std::string>())
        if (symbols.contains(node))
        {
            file = node + file;
            break;
        }
        nodes.push_back(node);
    }
    parsedObject = true;
    return nodes;
}
auto DbcParser::parseSignal() -> Core::DbcSignalDescription
{
    std::string signalName;
    bool valueType = false;
    std::string unit;
    std::list<std::string> receivers;
    bool multiplexor = false;
    int multiplexerSwitchValue = -1;
    uint startBit;
    uint signalSize;
    uint byteOrderInt;
    double factor;
    double offset;
    double minimum;
    double maximum;

    eraseSpaces();
    if (!file.starts_with("SG_"))
    {
        parsedObject = false;
        return {};
    }
    file = file.substr(3);
    signalName = parseCIdentifier();
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        if (file.starts_with("M"))
        {
            multiplexor = true;
            file = file.substr(1);
        } else if (file.starts_with("m"))
        {
            file = file.substr(1);
            multiplexerSwitchValue = parseUInt();
        }
    }
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    startBit = parseUInt();
    eraseSpaces();
    if (!file.starts_with("|"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    signalSize = parseUInt();
    eraseSpaces();
    if (!file.starts_with("@"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    byteOrderInt = parseUInt();
    eraseSpaces();
    if (file.starts_with("+"))
    {
        valueType = false;
        file = file.substr(1);
    } else if (file.starts_with("-"))
    {
        valueType = true;
        file = file.substr(1);
    } else
    {
        parsingValid = false;
    }
    eraseSpaces();
    if (!file.starts_with("("))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    factor = parseDouble();
    eraseSpaces();
    if (!file.starts_with(","))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    offset = parseDouble();
    eraseSpaces();
    if (!file.starts_with(")"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    eraseSpaces();
    if (!file.starts_with("["))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    minimum = parseDouble();
    eraseSpaces();
    if (!file.starts_with("|"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    maximum = parseDouble();
    eraseSpaces();
    if (!file.starts_with("]"))
    {
        parsingValid = false;
    }
    file = file.substr(1);
    unit = parseString();
    while (true)
    {
        std::string receiver = parseCIdentifier();
        if (!parsingValid)
        {
            break;
        }
        receivers.push_back(receiver);
        eraseSpaces();
        if (!file.starts_with(","))
        {
            break;
        }
        file = file.substr(1);
    }
    if (!parsingValid || !(byteOrderInt == 0 || byteOrderInt == 1))
    {
        parsedObject = false;
        parsingValid = false;
        return {};
    }
    parsedObject = true;
    Core::DbcSignalDescription signalDescription{.signalName = signalName,
                                                 .multiplexer = multiplexor,
                                                 .multiplexedBy = multiplexerSwitchValue,
                                                 .startBit = startBit,
                                                 .signalSize = signalSize,
                                                 .byteOrder = (byteOrderInt == 1),
                                                 .valueType = valueType,
                                                 .factor = factor,
                                                 .offset = offset,
                                                 .minimum = minimum,
                                                 .maximum = maximum,
                                                 .unit = unit,
                                                 .receivers = receivers};

    calculatePhysicalRange(signalDescription);

    return signalDescription;
}
auto DbcParser::parseSignalValue() -> Core::DbcSignalValueDescription
{
    uint messageId;
    std::string signalName;
    std::list<Core::DbcValueDescription> valueDescriptions;

    eraseSpaces();
    if (!file.starts_with("VAL_"))
    {
        parsedObject = false;
        return {};
    }
    file = file.substr(4);
    messageId = parseUInt();
    signalName = parseCIdentifier();
    while (true)
    {
        eraseSpaces();
        if (file.starts_with(";"))
        {
            file = file.substr(1);
            break;
        }
        const Core::DbcValueDescription valueDescription = parseValueDescription();
        if (!parsingValid)
        {
            break;
        }
        valueDescriptions.push_back(valueDescription);
    }
    IF_PARSING_INVALID_RETURN(Core::DbcSignalValueDescription())
    parsedObject = true;
    return Core::DbcSignalValueDescription{
        .messageId = messageId, .signalName = signalName, .signalDescriptions = valueDescriptions};
}
auto DbcParser::parseValueDescription() -> Core::DbcValueDescription
{
    const double value = parseDouble();
    const std::string meaning = parseString();
    IF_PARSING_INVALID_RETURN(Core::DbcValueDescription())
    return Core::DbcValueDescription{.value = value, .meaning = meaning};
}
auto DbcParser::parseVersion() -> std::string
{
    eraseSpaces();
    if (!file.starts_with("VERSION"))
    {
        parsedObject = false;
        return "";
    }
    file = file.substr(7);
    parsedObject = true;
    return parseString();
}
auto DbcParser::parseAttributeDefault() -> bool
{
    eraseSpaces();
    if (!file.starts_with("BA_DEF_DEF_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
auto DbcParser::parseAttributeDefinition() -> bool
{
    eraseSpaces();
    if (!file.starts_with("BA_DEF_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
auto DbcParser::parseAttributeValue() -> bool
{
    eraseSpaces();
    if (!file.starts_with("BA_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
void DbcParser::parseEnvironmentVariableData()
{
    eraseSpaces();
    if (!file.starts_with("ENVVAR_DATA_"))
    {
        parsedObject = false;
        return;
    }
    parsedObject = true;
    truncateToNextSemicolon();
}
auto DbcParser::parseEnvironmentVariable() -> bool
{
    eraseSpaces();
    if (!file.starts_with("EV_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
void DbcParser::parseEnvironmentVariableValueDefinitions()
{
    eraseSpaces();
    if (!file.starts_with("VAL_"))
    {
        parsedObject = false;
        return;
    }
    parsedObject = true;
    truncateToNextSemicolon();
}
auto DbcParser::parseMessageTransmitter() -> bool
{
    eraseSpaces();
    if (!file.starts_with("BO_TX_BU_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
void DbcParser::parseNewSymbols()
{
    eraseSpaces();
    if (!file.starts_with("NS_"))
    {
        parsedObject = false;
        return;
    }
    file = file.substr(3);
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        parsingValid = false;
        parsedObject = false;
        return;
    }
    file = file.substr(1);
    while (true)
    {
        const std::string symbol = parseCIdentifier();
        IF_PARSING_INVALID_RETURN()
        if (file.starts_with(":"))
        {
            file = symbol + file;
            break;
        }
    }
}
void DbcParser::parseSignalExtendedValueTypeList()
{
    eraseSpaces();
    if (!file.starts_with("SIG_VALTYPE_"))
    {
        parsedObject = false;
        return;
    }
    parsedObject = true;
    truncateToNextSemicolon();
}
void DbcParser::parseSignalGroup()
{
    eraseSpaces();
    if (!file.starts_with("SIG_GROUP_"))
    {
        parsedObject = false;
        return;
    }
    parsedObject = true;
    truncateToNextSemicolon();
}
auto DbcParser::parseSignalType() -> bool
{
    eraseSpaces();
    if (!file.starts_with("SGTYPE_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
auto DbcParser::parseSignalTypeReference() -> bool
{
    eraseSpaces();
    if (!file.starts_with("SGTYPE_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
auto DbcParser::parseValueTable() -> bool
{
    eraseSpaces();
    if (!file.starts_with("VAL_TABLE_"))
    {
        parsedObject = false;
        return false;
    }
    parsedObject = true;
    return truncateToNextSemicolon();
}
void DbcParser::parseBitTiming()
{
    eraseSpaces();
    if (!file.starts_with("BS_:"))
    {
        parsingValid = false;
        parsedObject = false;
        return;
    }
    file = file.substr(4);
    eraseSpaces();
    if (file.starts_with("BU_"))
    {
        parsingValid = true;
        parsedObject = true;
        return;
    }
    parseUInt();
    if (!parsingValid)
    {
        parsingValid = true;
        parsedObject = false;
        return;
    }
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        parsingValid = false;
        parsedObject = false;
        return;
    }
    file = file.substr(1);
    eraseSpaces();
    parseUInt();
    eraseSpaces();
    if (!file.starts_with(","))
    {
        parsingValid = false;
        parsedObject = false;
        return;
    }
    file = file.substr(1);
    parsedObject = true;
    parseUInt();
}
void DbcParser::eraseSpaces()
{
    while (!file.empty() && isspace(static_cast<unsigned char>(file.front())))
    {
        file = file.substr(1);
    }
}
auto DbcParser::parseCIdentifier() -> std::string
{
    eraseSpaces();
    const size_t pos = file.find_first_of(" :;,");
    if (pos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return "";
    }
    std::string identifier = file.substr(0, pos);
    if (!regex_match(identifier, C_IDENTIFIER_REGEX))
    {
        parsingValid = false;
        parsedObject = false;
        return "";
    }
    file = file.substr(pos);
    parsingValid = true;
    parsedObject = true;
    return identifier;
}
auto DbcParser::parseString() -> std::string
{
    eraseSpaces();
    if (!file.starts_with("\""))
    {
        parsingValid = false;
        parsedObject = false;
        return "";
    }
    file = file.substr(1);
    const int hyphenPos = file.find('\"');
    if (hyphenPos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return "";
    }
    std::string string = file.substr(0, hyphenPos);
    file = file.substr(hyphenPos + 1);
    parsingValid = true;
    parsedObject = true;
    return string;
}
auto DbcParser::parseDouble() -> double
{
    eraseSpaces();
    const size_t pos = file.find_first_of(" :;,|]@)");
    if (pos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    const std::string possibleDouble = file.substr(0, pos);
    if (!std::regex_match(possibleDouble, DOUBLE_REGEX))
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    file = file.substr(pos);
    parsingValid = true;
    parsedObject = true;
    return strtod(possibleDouble.c_str(), nullptr);
}
auto DbcParser::parseInt() -> int
{
    eraseSpaces();
    const size_t pos = file.find_first_of(" :;,|]@)");
    if (pos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    const std::string possibleInt = file.substr(0, pos);
    if (!std::regex_match(possibleInt, INT_REGEX))
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    file = file.substr(pos);
    parsingValid = true;
    parsedObject = true;
    try
    {
        return std::stoi(possibleInt);
    } catch (...)
    {
        LOG_ERR(1, "Error while parsing integer in DBC: %s", possibleInt.c_str());
        parsingValid = false;
        return 0;
    }
}
auto DbcParser::parseUInt() -> uint
{
    eraseSpaces();
    const size_t pos = file.find_first_of(" :;,|]+-@)");
    if (pos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    const std::string possibleInt = file.substr(0, pos);
    if (!std::regex_match(possibleInt, UINT_REGEX))
    {
        parsingValid = false;
        parsedObject = false;
        return 0;
    }
    file = file.substr(pos);
    parsingValid = true;
    parsedObject = true;
    try
    {
        return std::stoi(possibleInt);
    } catch (...)
    {
        LOG_ERR(1, "Error while parsing unsigned integer in DBC: %s", possibleInt.c_str());
        parsingValid = false;
        return 0;
    }
}
auto DbcParser::truncateToNextSemicolon() -> bool
{
    const size_t semicolonPos = file.find(';');
    if (semicolonPos == std::string::npos)
    {
        parsingValid = false;
        parsedObject = false;
        return false;
    }
    file = file.substr(semicolonPos + 1);
    parsedObject = true;
    return true;
}

void DbcParser::calculatePhysicalRange(Core::DbcSignalDescription& signal)
{
    if (signal.minimum == 0.0 && signal.maximum == 0.0)
    {
        unsigned long long rawMax;
        long long rawMin;
        if (signal.valueType)
        {
            rawMax = (1ULL << (signal.signalSize - 1)) - 1;
            rawMin = -(1LL << (signal.signalSize - 1));
        } else
        {
            rawMin = 0;
            rawMax = (1ULL << signal.signalSize) - 1;
        }
        signal.minimum = static_cast<double>(rawMin) * signal.factor + signal.offset;
        signal.maximum = static_cast<double>(rawMax) * signal.factor + signal.offset;
    } else if (signal.minimum >= signal.maximum)
    {
        LOG_WRN("CanHandler", "Signal '{}' has invalid range [{:.2f}|{:.2f}]",
                signal.signalName.c_str(), signal.minimum, signal.maximum);
    }
}

}  // namespace CanHandler