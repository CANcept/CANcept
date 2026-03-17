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
