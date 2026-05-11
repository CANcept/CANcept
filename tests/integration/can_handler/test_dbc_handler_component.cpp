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

#include <filesystem>
#include <fstream>
#include <memory>

#include "can_handler/dbc_handler/dbc_handler.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/mock_event_broker.hpp"
using namespace CanHandler;
namespace {
auto createTempFile(const std::string& content) -> std::filesystem::path
{
    const auto path =
        std::filesystem::temp_directory_path() /
        std::filesystem::path("canbusmanager_file_parser_test_" +
                              std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                              ".tmp");
    std::ofstream file(path);
    file << content;
    file.close();
    return path;
}
const std::string kMinimalValidDbc =
    "VERSION \"1.0\" "
    "NS_ : "
    "BS_: "
    "BU_: ECU1 ECU2 "
    "BO_ 100 TestMessage: 8 ECU1 "
    "SG_ TestSignal : 0|8@1+ (1,0) [0|255] \"V\" ECU2 ";
const std::string kInvalidDbc =
    "VERSION \"1.0\" "
    "NS_ : "
    "BS_: "
    "BU_: ECU1 ECU2 "
    "BO_ 300 InvalidMessage: 8 ECU1 "
    "SG_ Broken : 0|8@2+ (1,0) [0|255] \"\" ECU2 ";
}  // namespace

class DbcHandlerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        dbcHandler = std::make_unique<DbcHandler>(*eventBroker);
        validDbcConnection =
            eventBroker->subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
                lastDbc = std::make_unique<Core::DbcConfig>(event.config);
                validDbcCounter++;
                std::unique_lock<std::mutex> lock(eventMutex);
                eventCV.notify_one();
            });
        invalidDbcConnection = eventBroker->subscribe<Core::DBCParseErrorEvent>(
            [this](const Core::DBCParseErrorEvent& event) {
                invalidDbcCounter++;
                std::unique_lock<std::mutex> lock(eventMutex);
                eventCV.notify_one();
            });
        eventBroker->publish(Core::AppStartedEvent{});
    }

    void TearDown() override
    {
        eventBroker->publish(Core::AppStoppedEvent{});
        validDbcConnection.release();
        invalidDbcConnection.release();
        dbcHandler.reset();
        eventBroker.reset();
    }

    void waitForEvent()
    {
        std::unique_lock<std::mutex> lock(eventMutex);
        eventCV.wait_until(
            lock, std::chrono::steady_clock::now() + std::chrono::seconds(10),
            [this]() -> bool { return validDbcCounter > 0 || invalidDbcCounter > 0; });
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<DbcHandler> dbcHandler;
    Core::Connection validDbcConnection;
    Core::Connection invalidDbcConnection;
    std::condition_variable eventCV{};
    std::mutex eventMutex{};
    int validDbcCounter = 0;
    int invalidDbcCounter = 0;
    std::unique_ptr<Core::DbcConfig> lastDbc;
};

TEST_F(DbcHandlerTest, HandlesValidDbc)
{
    const auto filePath = createTempFile(kMinimalValidDbc);
    EXPECT_NO_THROW(eventBroker->publish(Core::ParseDBCRequestEvent(filePath)));
    waitForEvent();
    EXPECT_EQ(validDbcCounter, 1);
    EXPECT_EQ(invalidDbcCounter, 0);
    EXPECT_EQ(lastDbc->messageDefinitions.front().messageId, 100);
}

TEST_F(DbcHandlerTest, HandlesInvalidDbc)
{
    const auto filePath = createTempFile(kInvalidDbc);
    EXPECT_NO_THROW(eventBroker->publish(Core::ParseDBCRequestEvent(filePath)));
    waitForEvent();
    EXPECT_EQ(validDbcCounter, 0);
    EXPECT_EQ(invalidDbcCounter, 1);
}