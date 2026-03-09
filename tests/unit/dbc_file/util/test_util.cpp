#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QString>

#include "dbc_file/constants.hpp"
#include "dbc_file/util/util.hpp"

// ============================================================================
// FILE VALIDATION TESTS
// ============================================================================

TEST(UtilsTest, IsValidFileChecksExtensionCaseInsensitive)
{
    // Happy Path
    EXPECT_TRUE(DbcFile::Util::isValidFile("/path/to/file.dbc"));
    EXPECT_TRUE(DbcFile::Util::isValidFile("file.dbc"));

    // Case Insensitive
    EXPECT_TRUE(DbcFile::Util::isValidFile("file.DBC"));
    EXPECT_TRUE(DbcFile::Util::isValidFile("file.DbC"));

    // Invalid extensions
    EXPECT_FALSE(DbcFile::Util::isValidFile("file.txt"));
    EXPECT_FALSE(DbcFile::Util::isValidFile("file.dbc.txt"));
    EXPECT_FALSE(DbcFile::Util::isValidFile("file"));  // No extension
}

TEST(DbcUtilsTest, CanAcceptDropValidatesCountAndExtension)
{
    // Case 1: Single valid file -> OK
    QList<QString> validList = {"test.dbc"};
    EXPECT_TRUE(DbcFile::Util::canAcceptDrop(validList));

    // Case 2: Two valid files -> Fail (only single file allowed)
    QList<QString> multiList = {"test.dbc", "test2.dbc"};
    EXPECT_FALSE(DbcFile::Util::canAcceptDrop(multiList));

    // Case 3: Single invalid file -> Fail
    QList<QString> invalidList = {"test.txt"};
    EXPECT_FALSE(DbcFile::Util::canAcceptDrop(invalidList));

    // Case 4: Empty list -> Fail
    QList<QString> emptyList;
    EXPECT_FALSE(DbcFile::Util::canAcceptDrop(emptyList));
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
    QModelIndex idxCol1 = DbcFile::Util::siblingAtColumnQt5(idxCol0, 1);

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

    EXPECT_EQ(DbcFile::Util::resolveMessageId(index), targetId);
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

    EXPECT_EQ(DbcFile::Util::resolveMessageId(index), displayId);
}

TEST(UtilsTest, ResolveMessageIdReturnsZeroIfEmpty)
{
    QStandardItemModel model;
    QStandardItem* item = new QStandardItem();
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);

    // Case 3: Neither role is set
    EXPECT_EQ(DbcFile::Util::resolveMessageId(index), 0u);
}
