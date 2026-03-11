#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QString>

#include "dbc_file/constants.hpp"
#include "dbc_file/util/util.hpp"
#include "tests/helpers/dbc_config_builder.hpp"

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
    auto* item = new QStandardItem("DisplayValue");
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
    auto* item = new QStandardItem();
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
    auto* item = new QStandardItem();
    model.appendRow(item);
    QModelIndex index = model.index(0, 0);

    // Case 3: Neither role is set
    EXPECT_EQ(DbcFile::Util::resolveMessageId(index), 0u);
}
using namespace TestHelpers;
// ============================================================================
// Filter option extraction tests
// ============================================================================

struct UnitScenario {
    std::string name;
    Core::DbcConfig config;
    QStringList expectedUnits;
};

class UnitExtractionTest : public ::testing::TestWithParam<UnitScenario>
{
};

TEST_P(UnitExtractionTest, ExtractsAndSortsUnitsCorrectly)
{
    const auto& p = GetParam();
    Core::DbcConfig config(p.config);

    QStringList result = DbcFile::Util::extractSignalUnits(config);

    ASSERT_EQ(result.size(), p.expectedUnits.size());
    for (int i = 0; i < result.size(); ++i)
    {
        EXPECT_EQ(result[i], p.expectedUnits[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnitScenarios, UnitExtractionTest,
    ::testing::Values(
        // Case 1: Simple cleanup
        UnitScenario{"DuplicatesAndEmpty",
                     DbcConfigBuilder()
                         .message(DbcMessageBuilder(1, "M")
                                      .signal(DbcSignalBuilder("S1").unit("V"))
                                      .signal(DbcSignalBuilder("S2").unit("V"))  // Dup
                                      .signal(DbcSignalBuilder("S3").unit("")))  // Empty
                         .build(),
                     {"V"}},
        // Case 2: Case Sensitivity & Sorting
        UnitScenario{"CaseSensitiveSort",
                     DbcConfigBuilder()
                         .message(DbcMessageBuilder(1, "M")
                                      .signal(DbcSignalBuilder("S1").unit("rpm"))
                                      .signal(DbcSignalBuilder("S2").unit("Bar"))
                                      .signal(DbcSignalBuilder("S3").unit("km/h")))
                         .build(),
                     {"Bar", "km/h", "rpm"}}),
    [](const ::testing::TestParamInfo<UnitScenario>& info) { return info.param.name; });

struct SenderScenario {
    std::string name;
    Core::DbcConfig config;
    QStringList expectedSenders;
};

class SenderExtractionTest : public ::testing::TestWithParam<SenderScenario>
{
};

TEST_P(SenderExtractionTest, ExtractsAndSortsSendersCorrectly)
{
    const auto& p = GetParam();
    Core::DbcConfig config(p.config);

    QStringList result = DbcFile::Util::extractSenders(config);

    ASSERT_EQ(result.size(), p.expectedSenders.size());
    for (int i = 0; i < result.size(); ++i)
    {
        EXPECT_EQ(result[i], p.expectedSenders[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    SenderScenarios, SenderExtractionTest,
    ::testing::Values(
        SenderScenario{"DedupesSenders",
                       DbcConfigBuilder()
                           .message(DbcMessageBuilder(1, "M1").transmitter("Engine"))
                           .message(DbcMessageBuilder(2, "M2").transmitter("Engine"))  // Dup
                           .message(DbcMessageBuilder(3, "M3").transmitter("ABS"))
                           .build(),
                       {"ABS", "Engine"}},
        SenderScenario{"IgnoresEmpty",
                       DbcConfigBuilder()
                           .message(DbcMessageBuilder(1, "M1").transmitter(""))
                           .message(DbcMessageBuilder(2, "M2").transmitter("Gateway"))
                           .build(),
                       {"Gateway"}}),
    [](const ::testing::TestParamInfo<SenderScenario>& info) { return info.param.name; });