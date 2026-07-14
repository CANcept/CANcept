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
#include <string>
#include <vector>

#include "core/service/serializer.hpp"
#include "gtest/gtest.h"

namespace {
auto tempFilePath(const std::string& suffix) -> std::filesystem::path
{
    return std::filesystem::temp_directory_path() /
           std::filesystem::path("canbusmanager_serializer_test_" +
                                 std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                                 suffix);
}
}  // namespace

TEST(SerializerTest, ToJsonThenFromJsonPreservesBoundValues)
{
    int number = 42;
    std::string text = "hello";
    std::vector<int> list = {1, 2, 3};

    Core::Serializer serializer;
    serializer.addItem("number", number);
    serializer.addItem("text", text);
    serializer.addItem("list", list);

    const auto json = serializer.toJson();

    int loadedNumber = 0;
    std::string loadedText;
    std::vector<int> loadedList;

    Core::Serializer loader;
    loader.addItem("number", loadedNumber);
    loader.addItem("text", loadedText);
    loader.addItem("list", loadedList);
    loader.fromJson(json);

    EXPECT_EQ(loadedNumber, number);
    EXPECT_EQ(loadedText, text);
    EXPECT_EQ(loadedList, list);
}

TEST(SerializerTest, FromJsonSkipsMissingKeys)
{
    int value = 7;

    Core::Serializer serializer;
    serializer.addItem("value", value);
    serializer.fromJson(nlohmann::json::object());

    EXPECT_EQ(value, 7);
}

TEST(SerializerTest, CustomEncodeDecodeClosuresRoundTrip)
{
    int source = 5;
    int destination = 0;

    Core::Serializer serializer;
    serializer.addItem(
        "doubled", [&source] { return nlohmann::json(source * 2); },
        [&destination](const nlohmann::json& json) { destination = json.get<int>(); });

    const auto json = serializer.toJson();
    serializer.fromJson(json);

    EXPECT_EQ(destination, 10);
}

TEST(SerializerTest, SaveToFileThenLoadFromFileRoundTrips)
{
    const auto path = tempFilePath(".json");

    int savedValue = 99;
    Core::Serializer saver;
    saver.addItem("value", savedValue);
    ASSERT_TRUE(saver.saveToFile(path));

    int loadedValue = 0;
    Core::Serializer loader;
    loader.addItem("value", loadedValue);
    ASSERT_TRUE(loader.loadFromFile(path));

    EXPECT_EQ(loadedValue, savedValue);

    std::filesystem::remove(path);
}

TEST(SerializerTest, LoadFromFileReturnsFalseForMissingFile)
{
    Core::Serializer loader;
    EXPECT_FALSE(loader.loadFromFile("/tmp/this_file_should_not_exist_123456789.json"));
}
