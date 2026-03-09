#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QString>

#include "core/util/dbc_utils.hpp"
#include "dbc_file/constants.hpp"

using namespace DbcFile::Util;
inline void PrintTo(const QString& qString, std::ostream* os)
{
    *os << "\"" << qString.toStdString() << "\"";
}

// ============================================================================
// 1. FORMATTING TESTS
// ============================================================================

TEST(DbcUtilsTest, FormatIdReturnsHexUpperCase)
{
    // Case 1: Standard ID (decimal 255 -> hex FF)
    EXPECT_EQ(DbcFile::Util::formatId(255), "0x0FF");

    // Case 2: Zero padding (decimal 10 -> hex A -> 00A)
    EXPECT_EQ(formatId(10), "0x00A");

    // Case 3: Zero
    EXPECT_EQ(formatId(0), "0x000");

    // Case 4: Larger values (decimal 4095 -> hex FFF)
    EXPECT_EQ(formatId(4095), "0xFFF");

    // Case 5: Ensure uppercase
    EXPECT_EQ(formatId(2748), "0xABC");  // 2748 dec = ABC hex
}

TEST(DbcUtilsTest, FormatNumberRemovesTrailingZeros)
{
    // Case 1: Integer as double
    EXPECT_EQ(formatNumber(40.0), "40");

    // Case 2: Exact decimal
    EXPECT_EQ(formatNumber(0.125), "0.125");

    // Case 3: Negative number
    EXPECT_EQ(formatNumber(-5.5), "-5.5");

    // Case 4: Zero
    EXPECT_EQ(formatNumber(0.0), "0");
}

TEST(DbcUtilsTest, FormatRangeReturnsBracketString)
{
    EXPECT_EQ(formatRange(0, 100), "[0, 100]");
    EXPECT_EQ(formatRange(5, 5), "[5, 5]");
}