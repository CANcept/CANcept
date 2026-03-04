#include <filesystem>
#include <fstream>
#include <memory>

#include "can_handler/dbc_handler/file_parser.hpp"
#include "gtest/gtest.h"

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
}  // namespace

TEST(FileParserTest, ReturnsNullptrForMissingFile)
{
    const auto result = std::unique_ptr<std::string>(
        FileParser::parseFile("/tmp/this_file_should_not_exist_123456789.dbc"));

    EXPECT_EQ(result, nullptr);
}

TEST(FileParserTest, ReadsFileAndConcatenatesLinesWithSpaces)
{
    const auto filePath = createTempFile("line1\nline2\nline3");

    const auto result = std::unique_ptr<std::string>(FileParser::parseFile(filePath.string()));

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, "line1 line2 line3 ");

    std::filesystem::remove(filePath);
}

TEST(FileParserTest, ReturnsEmptyStringForEmptyFile)
{
    const auto filePath = createTempFile("");

    const auto result = std::unique_ptr<std::string>(FileParser::parseFile(filePath.string()));

    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->empty());

    std::filesystem::remove(filePath);
}
