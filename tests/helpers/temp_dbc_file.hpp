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

#include <QString>
#include <fstream>
#include <string>
#include <string_view>

namespace TestHelpers {

constexpr std::string_view kMinimalDbcContent =
    "VERSION \"1.0\"\n"
    "NS_ :\n"
    "BS_:\n"
    "BU_: TestNode\n"
    "BO_ 291 TestMessage: 8 TestNode\n"
    "SG_ TestSignal : 7|8@1+ (1,0) [0|255] \"units\" TestNode\n";

constexpr uint32_t kTestMsgId = 0x123;
inline const QString kTestSignalName = QStringLiteral("TestSignal");

inline auto makeTempDbcFile(const std::string& path = "/tmp/test_system.dbc") -> std::string
{
    std::ofstream out(path);
    out << kMinimalDbcContent;
    return path;
}

}  // namespace TestHelpers
