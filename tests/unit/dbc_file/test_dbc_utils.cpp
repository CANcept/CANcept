#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QString>

#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_utils.hpp"
#include "dbc_file/model/dbc_roles.hpp"

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

// ============================================================================
// 2. FILE VALIDATION TESTS
// ============================================================================

TEST(DbcUtilsTest, IsValidFileChecksExtensionCaseInsensitive)
{
    // Happy Path
    EXPECT_TRUE(isValidFile("/path/to/file.dbc"));
    EXPECT_TRUE(isValidFile("file.dbc"));

    // Case Insensitive
    EXPECT_TRUE(isValidFile("file.DBC"));
    EXPECT_TRUE(isValidFile("file.DbC"));

    // Invalid extensions
    EXPECT_FALSE(isValidFile("file.txt"));
    EXPECT_FALSE(isValidFile("file.dbc.txt"));
    EXPECT_FALSE(isValidFile("file"));  // No extension
}

TEST(DbcUtilsTest, CanAcceptDropValidatesCountAndExtension)
{
    // Case 1: Single valid file -> OK
    QList<QString> validList = {"test.dbc"};
    EXPECT_TRUE(canAcceptDrop(validList));

    // Case 2: Two valid files -> Fail (only single file allowed)
    QList<QString> multiList = {"test.dbc", "test2.dbc"};
    EXPECT_FALSE(canAcceptDrop(multiList));

    // Case 3: Single invalid file -> Fail
    QList<QString> invalidList = {"test.txt"};
    EXPECT_FALSE(canAcceptDrop(invalidList));

    // Case 4: Empty list -> Fail
    QList<QString> emptyList;
    EXPECT_FALSE(canAcceptDrop(emptyList));
}

// ============================================================================
// 3. MODEL INDEX HELPER TESTS
// ============================================================================

TEST(DbcUtilsTest, SiblingAtColumnReturnsCorrectIndex)
{
    // Setup: Create a small 1x2 model
    QStandardItemModel model(1, 2);
    QModelIndex idxCol0 = model.index(0, 0);

    // Act: Get sibling at column 1
    QModelIndex idxCol1 = siblingAtColumnQt5(idxCol0, 1);

    // Assert
    EXPECT_TRUE(idxCol1.isValid());
    EXPECT_EQ(idxCol1.row(), 0);
    EXPECT_EQ(idxCol1.column(), 1);
    EXPECT_EQ(idxCol1.model(), &model);
}

TEST(DbcUtilsTest, ResolveMessageIdPrioritizesRoleId)
{
    QStandardItemModel model;
    QStandardItem* item = new QStandardItem("DisplayValue");
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);

    constexpr uint targetId = 123;
    constexpr uint displayId = 456;

    // Case 1: Role_Id is set
    model.setData(index, targetId, DbcFile::DbcRoles::Role_Id);
    // Set DisplayRole to something else to ensure we picked Role_Id
    model.setData(index, QString::number(displayId), Qt::DisplayRole);

    EXPECT_EQ(resolveMessageId(index), targetId);
}

TEST(DbcUtilsTest, ResolveMessageIdFallsBackToDisplayRole)
{
    QStandardItemModel model;
    QStandardItem* item = new QStandardItem();
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);

    constexpr uint displayId = 789;

    // Case 2: Role_Id is missing/zero, verify fallback to DisplayRole
    // Note: QStandardItem initially has no data for custom roles, so toUInt() returns 0
    model.setData(index, QString::number(displayId), Qt::DisplayRole);

    EXPECT_EQ(resolveMessageId(index), displayId);
}

TEST(DbcUtilsTest, ResolveMessageIdReturnsZeroIfEmpty)
{
    QStandardItemModel model;
    QStandardItem* item = new QStandardItem();
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);

    // Case 3: Neither role is set
    EXPECT_EQ(resolveMessageId(index), 0u);
}