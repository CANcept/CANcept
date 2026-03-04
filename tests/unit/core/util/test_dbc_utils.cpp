#include <gtest/gtest.h>
#include <QStandardItemModel>
#include "core/util/dbc_utils.hpp"

using namespace Core::Util;

// =======================
// formatId Tests
// =======================
TEST(DbcUtils, FormatIdBasic)
{
    EXPECT_EQ(formatId(0), QStringLiteral("0x000"));
    EXPECT_EQ(formatId(255), QStringLiteral("0x0FF"));
    EXPECT_EQ(formatId(4095), QStringLiteral("0xFFF"));
    EXPECT_EQ(formatId(65535), QStringLiteral("0xFFFF"));
}

// =======================
// formatNumber Tests
// =======================
TEST(DbcUtils, FormatNumberBasicAndEdge)
{
    EXPECT_EQ(formatNumber(40.0), QStringLiteral("40"));
    EXPECT_EQ(formatNumber(0.125), QStringLiteral("0.125"));
    EXPECT_EQ(formatNumber(123456.7890123), QStringLiteral("123456.7890123"));
    EXPECT_EQ(formatNumber(0.0), QStringLiteral("0"));
    EXPECT_EQ(formatNumber(-42.5), QStringLiteral("-42.5"));
    EXPECT_EQ(formatNumber(1e-10), QStringLiteral("1e-10"));
    EXPECT_EQ(formatNumber(1e10), QStringLiteral("1e+10"));
}

// =======================
// formatRange Tests
// =======================
TEST(DbcUtils, FormatRangeBasic)
{
    EXPECT_EQ(formatRange(0, 100), QStringLiteral("[0, 100]"));
    EXPECT_EQ(formatRange(255, 1023), QStringLiteral("[255, 1023]"));
}

// =======================
// siblingAtColumnQt5 Tests
// =======================
TEST(DbcUtils, SiblingAtColumnQt5ReturnsCorrectIndex)
{
    QStandardItemModel model;
    model.appendRow(new QStandardItem("Row0Col0"));
    model.setHorizontalHeaderLabels({"Col0", "Col1", "Col2"});

    QModelIndex idx0 = model.index(0, 0);
    QModelIndex sibling = siblingAtColumnQt5(idx0, 2);

    EXPECT_EQ(sibling.row(), 0);
    EXPECT_EQ(sibling.column(), 2);
}

// =======================
// isValidFile Tests
// =======================
TEST(DbcUtils, IsValidFileWorks)
{
    EXPECT_TRUE(isValidFile("test.dbc"));
    EXPECT_TRUE(isValidFile("/path/to/file.DBC"));
    EXPECT_FALSE(isValidFile("not_a_dbc.txt"));
    EXPECT_FALSE(isValidFile(""));
}

// =======================
// canAcceptDrop Tests
// =======================
TEST(DbcUtils, CanAcceptDropSingleValidFile)
{
    QList<QString> files = {"test.dbc"};
    EXPECT_TRUE(canAcceptDrop(files));

    files = {"test.dbc", "other.dbc"};
    EXPECT_FALSE(canAcceptDrop(files));

    files = {"not_valid.txt"};
    EXPECT_FALSE(canAcceptDrop(files));

    files.clear();
    EXPECT_FALSE(canAcceptDrop(files));
}

// =======================
// resolveMessageId Tests
// =======================
TEST(DbcUtils, ResolveMessageIdFallsBackToDisplayRole)
{
    QStandardItemModel model;
    QStandardItem* item = new QStandardItem("123");
    item->setData(0, DbcFile::DbcRoles::Role_Id);  // Role_Id = 0
    model.appendRow(item);

    QModelIndex idx = model.index(0, 0);

    EXPECT_EQ(resolveMessageId(idx), 123u);  // Fallback to DisplayRole
}