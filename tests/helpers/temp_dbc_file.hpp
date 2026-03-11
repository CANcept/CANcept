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
    "SG_ TestSignal : 0|8@1+ (1,0) [0|255] \"units\" TestNode\n";

constexpr uint32_t kTestMsgId = 0x123;
inline const QString kTestSignalName = QStringLiteral("TestSignal");

inline auto makeTempDbcFile(const std::string& path = "/tmp/test_system.dbc") -> std::string
{
    std::ofstream out(path);
    out << kMinimalDbcContent;
    return path;
}

}  // namespace TestHelpers
