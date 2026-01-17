//
// Created by flori on 29.12.2025.
//

#ifndef CANBUSMANAGER_DBC_DTO_HPP
#define CANBUSMANAGER_DBC_DTO_HPP
#include <list>
#include <string>
namespace Core {
struct DbcSignalDescription {
    std::string signalName;
    bool multiplexer;
    std::string multiplexedBy;
    uint startBit;
    uint signalSize;
    bool byteOrder;
    bool valueType;
    double factor;
    double offset;
    double minimum;
    double maximum;
    std::string unit;
    std::list<std::string> receivers;
};
struct DbcMessageDescription {
    uint messageId;
    std::string messageName;
    uint messageSize;
    std::string transmitterName;
    std::list<DbcSignalDescription> signalDescriptions;
};
struct DbcValueDescription {
    double value;
    std::string meaning;
};
struct DbcSignalValueDescription {
    uint messageId;
    std::string signalName;
    std::list<DbcValueDescription> signalDescriptions;
};
struct DbcMetaData
{
    std::string version;
    std::string fileName;
};
struct DbcConfig {
    std::list<std::string> nodeDefinitions;
    std::list<DbcMessageDescription> messageDefinitions;
    std::list<DbcSignalValueDescription> signalValueDescriptions;
    std::list<std::string> comments;
    DbcMetaData metaData;
};
}  // namespace Core
#endif  // CANBUSMANAGER_DBC_DTO_HPP
