//
// Created by flori on 21.01.2026.
//
#include "dbc_parser.hpp"

#include <iostream>
#include <regex>
namespace CanHandler {
void DbcParser::provideNewFile(const std::string& newFile)
{
    file = newFile;
}
auto DbcParser::parseDbc() -> Core::DbcConfig*
{
    lastParseValid = true;
    const auto config = new Core::DbcConfig();
    std::string* version = parseVersion();
    if (version == nullptr)
    {
        config->metaData.version = "";
    } else
    {
        config->metaData.version = *version;
    }
    parseNewSymbols();
    if (!lastParseValid)
    {
        delete config;
        return nullptr;
    }
    parseBitTiming();
    if (!lastParseValid)
    {
        delete config;
        return nullptr;
    }
    std::list<std::string>* nodes = parseNodes();
    if (!lastParseValid || nodes == nullptr)
    {
        delete config;
        delete nodes;
        return nullptr;
    }
    config->nodeDefinitions = *nodes;
    delete nodes;
    while (parseValueTable())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (true)
    {
        const Core::DbcMessageDescription* messageDescription = parseMessage();
        if (messageDescription == nullptr)
        {
            if (!lastParseValid)
            {
                delete config;
                return nullptr;
            }
            break;
        }
        config->messageDefinitions.push_back(*messageDescription);
        delete messageDescription;
    }
    while (parseMessageTransmitter())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (parseEnvironmentVariable())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    parseEnvironmentVariableData();
    if (!lastParseValid)
    {
        delete config;
        return nullptr;
    }
    while (parseSignalType())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (true)
    {
        const std::string* comment = parseComment();
        if (comment == nullptr)
        {
            if (!lastParseValid)
            {
                delete config;
                return nullptr;
            }
            break;
        }
        config->comments.push_back(*comment);
        delete comment;
    }
    while (parseAttributeDefinition())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (parseAttributeDefault())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (parseAttributeValue())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    while (true)
    {
        const Core::DbcSignalValueDescription* valueDescription = parseSignalValue();
        if (valueDescription == nullptr)
        {
            if (!lastParseValid)
            {
                delete config;
                return nullptr;
            }
            break;
        }
        if (valueDescription->messageId == static_cast<uint>(-1))
        {
            continue;
        }
        config->signalValueDescriptions.push_back(*valueDescription);
        delete valueDescription;
    }
    while (parseSignalTypeReference())
    {
        if (!lastParseValid)
        {
            delete config;
            return nullptr;
        }
    }
    parseSignalGroup();
    if (!lastParseValid)
    {
        delete config;
        return nullptr;
    }
    parseSignalExtendedValueTypeList();
    if (!lastParseValid)
    {
        delete config;
        return nullptr;
    }
    eraseSpaces();
    if (file != "")
    {
        delete config;
        return nullptr;
    }
    return config;
}
auto DbcParser::parseComment() -> std::string*
{
    eraseSpaces();
    if (!file.starts_with("CM_"))
    {
        return nullptr;
    }
    file = file.substr(3);
    std::string* comment = parseString();
    if (comment == nullptr)
    {
        lastParseValid = false;
        delete comment;
        return nullptr;
    }
    eraseSpaces();
    if (!file.starts_with(";"))
    {
        delete comment;
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(1);
    return comment;
}
auto DbcParser::parseMessage() -> Core::DbcMessageDescription*
{
    eraseSpaces();
    if (!file.starts_with("BO_"))
    {
        return nullptr;
    }
    file = file.substr(3);
    const uint* messageId = parseUInt();
    if (messageId == nullptr)
    {
        lastParseValid = false;
        return nullptr;
    }
    const std::string* messageName = parseCIdentifier();
    if (messageName == nullptr)
    {
        lastParseValid = false;
        delete messageId;
        return nullptr;
    }
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        lastParseValid = false;
        delete messageId;
        delete messageName;
        return nullptr;
    }
    file = file.substr(1);
    const uint* messageSize = parseUInt();
    if (messageSize == nullptr)
    {
        lastParseValid = false;
        delete messageId;
        delete messageName;
        return nullptr;
    }
    const std::string* transmitter = parseCIdentifier();
    if (transmitter == nullptr)
    {
        lastParseValid = false;
        delete messageId;
        delete messageName;
        delete messageSize;
        return nullptr;
    }
    Core::DbcMessageDescription* messageDescription =
        new Core::DbcMessageDescription{.messageId = *messageId,
                                        .messageName = *messageName,
                                        .messageSize = *messageSize,
                                        .transmitterName = *transmitter};
    while (true)
    {
        const Core::DbcSignalDescription* signalDescription = parseSignal();
        if (signalDescription == nullptr)
        {
            delete signalDescription;
            if (!lastParseValid)
            {
                delete messageId;
                delete messageName;
                delete messageSize;
                delete transmitter;
                delete messageDescription;
                return nullptr;
            }
            break;
        }
        messageDescription->signalDescriptions.push_back(*signalDescription);
        delete signalDescription;
    }
    delete messageId;
    delete messageName;
    delete messageSize;
    delete transmitter;
    lastParseValid = true;
    return messageDescription;
}
auto DbcParser::parseNodes() -> std::list<std::string>*
{
    eraseSpaces();
    if (!file.starts_with("BU_:"))
    {
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(4);
    std::list<std::string>* nodes = new std::list<std::string>;
    while (true)
    {
        std::string* node = parseCIdentifier();
        if (node == nullptr)
        {
            delete node;
            delete nodes;
            lastParseValid = false;
            return nullptr;
        }
        if (symbols.contains(*node))
        {
            file = *node + file;
            delete node;
            break;
        }
        nodes->push_back(*node);
        delete node;
    }
    return nodes;
}
auto DbcParser::parseSignal() -> Core::DbcSignalDescription*
{
    std::string* signalName;
    bool valueType = false;
    std::string* unit;
    std::list<std::string> receivers;
    bool multiplexor = false;
    int* multiplexerSwitchValue = new int(-1);
    uint* startBit;
    uint* signalSize;
    uint* byteOrderInt;
    double* factor;
    double* offset;
    double* minimum;
    double* maximum;

    eraseSpaces();
    if (!file.starts_with("SG_"))
    {
        delete multiplexerSwitchValue;
        return nullptr;
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
            delete multiplexerSwitchValue;
            multiplexerSwitchValue = reinterpret_cast<int*>(parseUInt());
        }
    }
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    startBit = parseUInt();
    eraseSpaces();
    if (!file.starts_with("|"))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    signalSize = parseUInt();
    eraseSpaces();
    if (!file.starts_with("@"))
    {
        lastParseValid = false;
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
        lastParseValid = false;
    }
    eraseSpaces();
    if (!file.starts_with("("))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    factor = parseDouble();
    eraseSpaces();
    if (!file.starts_with(","))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    offset = parseDouble();
    eraseSpaces();
    if (!file.starts_with(")"))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    eraseSpaces();
    if (!file.starts_with("["))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    minimum = parseDouble();
    eraseSpaces();
    if (!file.starts_with("|"))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    maximum = parseDouble();
    eraseSpaces();
    if (!file.starts_with("]"))
    {
        lastParseValid = false;
    }
    file = file.substr(1);
    unit = parseString();
    while (true)
    {
        std::string* receiver = parseCIdentifier();
        if (receiver == nullptr)
        {
            lastParseValid = false;
            delete receiver;
            break;
        }
        receivers.push_back(*receiver);
        delete receiver;
        eraseSpaces();
        if (!file.starts_with(","))
        {
            break;
        }
        file = file.substr(1);
    }
    if (!lastParseValid || signalName == nullptr  || unit == nullptr ||
        multiplexerSwitchValue == nullptr || startBit == nullptr || signalSize == nullptr ||
        byteOrderInt == nullptr || factor == nullptr || offset == nullptr || minimum == nullptr ||
        maximum == nullptr ||
        !(*byteOrderInt == 0 || *byteOrderInt == 1))
    {
        delete signalName;
        delete unit;
        delete multiplexerSwitchValue;
        delete startBit;
        delete signalSize;
        delete byteOrderInt;
        delete factor;
        delete offset;
        delete minimum;
        delete maximum;
        lastParseValid = false;
        return nullptr;
    }
    Core::DbcSignalDescription* signalDescription =
        new Core::DbcSignalDescription{.signalName = *signalName,
                                       .multiplexer = multiplexor,
                                       .multiplexedBy = *multiplexerSwitchValue,
                                       .startBit = *startBit,
                                       .signalSize = *signalSize,
                                       .byteOrder = (*byteOrderInt == 1),
                                       .valueType = valueType,
                                       .factor = *factor,
                                       .offset = *offset,
                                       .minimum = *minimum,
                                       .maximum = *maximum,
                                       .unit = *unit,
                                       .receivers = receivers};
    delete signalName;
    delete unit;
    delete multiplexerSwitchValue;
    delete startBit;
    delete signalSize;
    delete byteOrderInt;
    delete factor;
    delete offset;
    delete minimum;
    delete maximum;
    return signalDescription;
}
auto DbcParser::parseSignalValue() -> Core::DbcSignalValueDescription*
{
    uint* messageId;
    std::string* signalName;
    std::list<Core::DbcValueDescription> valueDescriptions;

    eraseSpaces();
    if (!file.starts_with("VAL_"))
    {
        return nullptr;
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
        const Core::DbcValueDescription* valueDescription = parseValueDescription();
        if (valueDescription == nullptr)
        {
            lastParseValid = false;
            delete valueDescription;
            break;
        }
        valueDescriptions.push_back(*valueDescription);
        delete valueDescription;
    }
    if (!lastParseValid || messageId == nullptr || signalName == nullptr)
    {
        lastParseValid = false;
        delete signalName;
        delete messageId;
        return nullptr;
    }
    Core::DbcSignalValueDescription* signalValueDescription =
        new Core::DbcSignalValueDescription{.messageId = *messageId,
                                            .signalName = *signalName,
                                            .signalDescriptions = valueDescriptions};
    delete signalName;
    delete messageId;
    return signalValueDescription;
}
auto DbcParser::parseValueDescription() -> Core::DbcValueDescription*
{
    const double* value = parseDouble();
    const std::string* meaning = parseString();
    if (value == nullptr || meaning == nullptr)
    {
        delete value;
        delete meaning;
        lastParseValid = false;
        return nullptr;
    }
    Core::DbcValueDescription* signalValueDescription =
        new Core::DbcValueDescription{.value = *value, .meaning = *meaning};
    delete meaning;
    delete value;
    return signalValueDescription;
}
auto DbcParser::parseVersion() -> std::string*
{
    eraseSpaces();
    if (!file.starts_with("VERSION"))
    {
        return nullptr;
    }
    file = file.substr(7);
    return parseString();
}
bool DbcParser::parseAttributeDefault()
{
    eraseSpaces();
    if (!file.starts_with("BA_DEF_DEF_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
bool DbcParser::parseAttributeDefinition()
{
    eraseSpaces();
    if (!file.starts_with("BA_DEF_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
bool DbcParser::parseAttributeValue()
{
    eraseSpaces();
    if (!file.starts_with("BA_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
void DbcParser::parseEnvironmentVariableData()
{
    eraseSpaces();
    if (!file.starts_with("ENVVAR_DATA_"))
    {
        return;
    }
    truncateToNextSemicolon();
}
bool DbcParser::parseEnvironmentVariable()
{
    eraseSpaces();
    if (!file.starts_with("EV_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
void DbcParser::parseEnvironmentVariableValueDefinitions()
{
    eraseSpaces();
    if (!file.starts_with("VAL_"))
    {
        return;
    }
    truncateToNextSemicolon();
}
bool DbcParser::parseMessageTransmitter()
{
    eraseSpaces();
    if (!file.starts_with("BO_TX_BU_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
void DbcParser::parseNewSymbols()
{
    eraseSpaces();
    if (!file.starts_with("NS_"))
    {
        return;
    }
    file = file.substr(3);
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        lastParseValid = false;
        return;
    }
    file = file.substr(1);
    while (true)
    {
        const std::string* symbol = parseCIdentifier();
        if (symbol == nullptr)
        {
            lastParseValid = true;
            delete symbol;
            return;
        }
        if (file.starts_with(":"))
        {
            file = *symbol + file;
            delete symbol;
            break;
        }
        delete symbol;
    }
}
void DbcParser::parseSignalExtendedValueTypeList()
{
    eraseSpaces();
    if (!file.starts_with("SIG_VALTYPE_"))
    {
        return;
    }
    truncateToNextSemicolon();
}
void DbcParser::parseSignalGroup()
{
    eraseSpaces();
    if (!file.starts_with("SIG_GROUP_"))
    {
        return;
    }
    truncateToNextSemicolon();
}
bool DbcParser::parseSignalType()
{
    eraseSpaces();
    if (!file.starts_with("SGTYPE_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
bool DbcParser::parseSignalTypeReference()
{
    eraseSpaces();
    if (!file.starts_with("SGTYPE_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
auto DbcParser::parseValueTable() -> bool
{
    eraseSpaces();
    if (!file.starts_with("VAL_TABLE_"))
    {
        return false;
    }
    return truncateToNextSemicolon();
}
void DbcParser::parseBitTiming()
{
    eraseSpaces();
    if (!file.starts_with("BS_:"))
    {
        lastParseValid = false;
        return;
    }
    file = file.substr(4);
    eraseSpaces();
    delete parseUInt();
    if (!lastParseValid)
    {
        lastParseValid = true;
        return;
    }
    eraseSpaces();
    if (!file.starts_with(":"))
    {
        lastParseValid = false;
        return;
    }
    file = file.substr(1);
    eraseSpaces();
    delete parseUInt();
    eraseSpaces();
    if (!file.starts_with(","))
    {
        lastParseValid = false;
        return;
    }
    file = file.substr(1);
    delete parseUInt();
}
void DbcParser::eraseSpaces()
{
    while (file.front() == ' ')
    {
        file.erase(file.begin());
    }
}
auto DbcParser::parseCIdentifier() -> std::string*
{
    eraseSpaces();
    if (!regex_match(file, std::regex("([A-Z]|[a-z]|_)(\\w)+( |:).*")))
    {
        lastParseValid = false;
        return nullptr;
    }
    const int pos = std::min(file.find(' '), file.find(':'));
    std::string* identifier = new std::string(file.substr(0, pos));
    file = file.substr(pos);
    lastParseValid = true;
    return identifier;
}
auto DbcParser::parseString() -> std::string*
{
    eraseSpaces();
    if (!std::regex_match(file, std::regex("\".*\".*")))
    {
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(1);
    const int hyphenPos = file.find('\"');
    std::string* string = new std::string(file.substr(0, hyphenPos));
    file = file.substr(hyphenPos + 1);
    lastParseValid = true;
    return string;
}
auto DbcParser::parseDouble() -> double*
{
    eraseSpaces();
    const int pos = std::min({file.find(' '), file.find(':'), file.find(';'), file.find(','), file.find('|'),
                  file.find(']'), file.find('@'), file.find(')')});
    if (pos == std::string::npos)
    {
        lastParseValid = false;
        return nullptr;
    }
    const std::string possibleDouble = file.substr(0, pos);
    if (!std::regex_match(possibleDouble, std::regex("(\\+|-)?\\d+(\\.\\d+)?")))
    {
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(pos);
    lastParseValid = true;
    return new double(strtod(possibleDouble.c_str(), nullptr));
}
auto DbcParser::parseInt() -> int*
{
    eraseSpaces();
    const int pos = std::min({file.find(' '), file.find(':'), file.find(';'), file.find(','), file.find('|'),
                  file.find(']'), file.find('@'), file.find(')')});
    if (pos == std::string::npos)
    {
        lastParseValid = false;
        return nullptr;
    }
    const std::string possibleInt = file.substr(0, pos);
    if (!std::regex_match(possibleInt, std::regex("(\\+|-)?\\d+")))
    {
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(pos);
    lastParseValid = true;
    return new int(std::stoi(possibleInt));
}
auto DbcParser::parseUInt() -> uint*
{
    eraseSpaces();
    const int pos =
        std::min({file.find(' '), file.find(':'), file.find(';'), file.find(','), file.find('|'),
                  file.find(']'), file.find('+'), file.find('-'), file.find('@'), file.find(')')});
    if (pos == std::string::npos)
    {
        lastParseValid = false;
        return nullptr;
    }
    const std::string possibleInt = file.substr(0, pos);
    if (!std::regex_match(possibleInt, std::regex("\\d+")))
    {
        lastParseValid = false;
        return nullptr;
    }
    file = file.substr(pos);
    lastParseValid = true;
    return new uint(std::stoi(possibleInt));
}
auto DbcParser::truncateToNextSemicolon() -> bool
{
    const int semicolonPos = file.find(';');
    if (semicolonPos == std::string::npos)
    {
        lastParseValid = false;
        return false;
    }
    file = file.substr(semicolonPos);
    return true;
}

}  // namespace CanHandler