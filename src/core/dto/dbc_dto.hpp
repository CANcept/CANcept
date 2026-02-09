//
// Created by flori on 29.12.2025.
//

#pragma once
#include <list>
#include <string>
namespace Core {
struct DbcSignalDescription {
    std::string signalName;
    bool multiplexer;
    /**
     * Value when active, = -1 if not multiplexed
     */
    int multiplexedBy;
    uint startBit;
    uint signalSize;
    /**
     * false = little endian, true = large endian
     */
    bool byteOrder;
    /**
     * false = unsigned, true = signed
     */
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
struct DbcMetaData {
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
