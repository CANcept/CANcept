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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <memory>

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_model.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/dbc_examples.hpp"

using namespace TestHelpers;
class DbcModelTestBase : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        model = std::make_unique<DbcFile::DbcModel>();
    }

    void TearDown() override
    {
        model.reset();
    }

    // --- Role Collections for Negative Testing ---
    const std::vector<int> signalRoles = {DbcFile::Role_Unit,      DbcFile::Role_StartBit,
                                          DbcFile::Role_BitLength, DbcFile::Role_Factor,
                                          DbcFile::Role_Offset,    DbcFile::Role_Min,
                                          DbcFile::Role_Max,       DbcFile::Role_ByteOrder,
                                          DbcFile::Role_ValueType, DbcFile::Role_Receivers};

    const std::vector<int> messageRoles = {
        DbcFile::Role_Dlc, DbcFile::Role_Sender
        // Role_Id treated separately because signals inherit
    };

    const std::vector<int> ecuRoles = {DbcFile::Role_EcuTotalSignals};

    /**
     * @brief Checks for invalid returns of all roles in given list for the given index.
     */
    void assertInvalidRoles(const QModelIndex& idx, const std::vector<int>& roles) const
    {
        for (int role : roles)
        {
            SCOPED_TRACE(testing::Message() << "Checking invalid role: " << role);
            EXPECT_FALSE(model->data(idx, role).isValid());
        }
    }

    /**
     * @brief Helper to get data easily from the model.
     */
    QVariant getVal(const QModelIndex& idx, int role = Qt::DisplayRole) const
    {
        return model->data(idx, role);
    }

    std::unique_ptr<DbcFile::DbcModel> model;
};

// ============================================================================
// 1. Structural & Qt Compliance Tests
// ============================================================================

TEST_F(DbcModelTestBase, Qt_ModelConsistency_Check)
{
    model->setDbcConfig(DbcExamples::vehicleSensors());
    QAbstractItemModelTester tester(model.get(),
                                    QAbstractItemModelTester::FailureReportingMode::Fatal);
}

TEST_F(DbcModelTestBase, Qt_Defensive_InvalidInputs)
{
    model->setDbcConfig(DbcExamples::simple());

    // 1. index() Out of Bounds
    EXPECT_FALSE(model->index(-1, 0, QModelIndex()).isValid());   // Negative Row
    EXPECT_FALSE(model->index(0, -1, QModelIndex()).isValid());   // Negative Col
    EXPECT_FALSE(model->index(999, 0, QModelIndex()).isValid());  // Row too high

    // 2. data() with invalid index
    EXPECT_FALSE(model->data(QModelIndex(), Qt::DisplayRole).isValid());

    // 3. parent() of invalid index
    EXPECT_FALSE(model->parent(QModelIndex()).isValid());

    // 4. parent() of Top-Level Item has to be invalid -> root
    QModelIndex topLvlIdx = model->index(0, 0, QModelIndex());
    EXPECT_FALSE(model->parent(topLvlIdx).isValid());
}

TEST_F(DbcModelTestBase, Hierarchy_BuildsCorrectly)
{
    auto config = DbcExamples::simple();
    model->setDbcConfig(config);

    // Level 0: Root (Overview + ECUs)
    // Simple Example: Overview + 1 ECU ("TestNode")
    EXPECT_EQ(model->rowCount(QModelIndex()), 2);

    // Check Overview (Row 0)
    QModelIndex overviewIdx = model->index(0, 0, QModelIndex());
    EXPECT_EQ(getVal(overviewIdx, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Overview);

    // Check ECU (Row 1)
    QModelIndex ecuIdx = model->index(1, 0, QModelIndex());
    EXPECT_EQ(getVal(ecuIdx).toString().toStdString(), "TestNode");
    EXPECT_EQ(getVal(ecuIdx, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Ecu);

    // Level 1: Messages (Children of ECU)
    EXPECT_EQ(model->rowCount(ecuIdx), 1);
    QModelIndex msgIdx = model->index(0, 0, ecuIdx);
    EXPECT_EQ(getVal(msgIdx).toString().toStdString(), "TestMessage");
    EXPECT_EQ(model->parent(msgIdx), ecuIdx);
    EXPECT_EQ(getVal(msgIdx, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Message);

    // Level 2: Signals (Children of Message)
    EXPECT_EQ(model->rowCount(msgIdx), 1);
    QModelIndex sigIdx = model->index(0, 0, msgIdx);
    EXPECT_EQ(getVal(sigIdx).toString().toStdString(), "TestSignal");
    EXPECT_EQ(model->parent(sigIdx), msgIdx);
    EXPECT_EQ(getVal(sigIdx, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Signal);
}

TEST_F(DbcModelTestBase, Structural_ColumnCount_ByType)
{
    model->setDbcConfig(DbcExamples::simple());
    QModelIndex overview = model->index(0, 0, QModelIndex());
    QModelIndex ecu = model->index(1, 0, QModelIndex());
    QModelIndex msg = model->index(0, 0, ecu);

    EXPECT_EQ(model->columnCount(overview), DbcFile::Constants::Columns::OvColumnCount);
    EXPECT_EQ(model->columnCount(ecu), DbcFile::Constants::Columns::MsgColumnCount);
    EXPECT_EQ(model->columnCount(msg), DbcFile::Constants::Columns::SignalColumnCount);
}

TEST_F(DbcModelTestBase, HasChildren_BehavesCorrectly)
{
    model->setDbcConfig(DbcExamples::simple());

    QModelIndex root = QModelIndex();
    QModelIndex overview = model->index(0, 0, root);
    QModelIndex ecu = model->index(1, 0, root);
    QModelIndex msg = model->index(0, 0, ecu);
    QModelIndex sig = model->index(0, 0, msg);

    EXPECT_TRUE(model->hasChildren(root));
    EXPECT_FALSE(model->hasChildren(overview));
    EXPECT_TRUE(model->hasChildren(ecu));
    EXPECT_TRUE(model->hasChildren(msg));
    EXPECT_FALSE(model->hasChildren(sig));
}

TEST_F(DbcModelTestBase, Api_RowCount_ReturnsZeroForParentColumnsGreaterThanZero)
{
    model->setDbcConfig(DbcExamples::simple());

    const QModelIndex overviewColumnOne = model->index(0, 1, QModelIndex());
    ASSERT_TRUE(overviewColumnOne.isValid());

    EXPECT_EQ(model->rowCount(overviewColumnOne), 0);
}
// ============================================================================
// 2. Item Specific Data Tests (Positive & Negative Paths)
// ============================================================================

TEST_F(DbcModelTestBase, OverviewDataIsCorrect)
{
    model->setDbcConfig(DbcExamples::simple());

    QModelIndex idx = model->index(0, 0, QModelIndex());
    ASSERT_TRUE(idx.isValid());

    // 1. Check Custom Roles
    EXPECT_EQ(getVal(idx, DbcFile::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Overview);
    EXPECT_EQ(getVal(idx, DbcFile::Role_ChildCount).toInt(),
              0);  // Overview has no children in tree logic (it's a leaf in root)

    // 2. Check DisplayRole for ALL Columns (0 to 5)
    // Col 0: Filename
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvFilename), Qt::DisplayRole)
            .toString(),
        "simple.dbc");
    // Col 1: Version
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvVersion), Qt::DisplayRole)
            .toString(),
        "1.0");
    // Col 2: Node Count
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvEcuCount), Qt::DisplayRole)
            .toInt(),
        1);
    // Col 3: Msg Count
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvMsgCount), Qt::DisplayRole)
            .toInt(),
        1);
    // Col 4: Sig Count
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvSigCount), Qt::DisplayRole)
            .toInt(),
        1);
    // Col 5: Orphans
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::OvOrphans), Qt::DisplayRole)
            .toInt(),
        0);

    // --- Negative Path ---
    assertInvalidRoles(idx, signalRoles);
    assertInvalidRoles(idx, messageRoles);
    assertInvalidRoles(idx, ecuRoles);
    EXPECT_FALSE(getVal(idx, DbcFile::Role_Id).isValid());
}

TEST_F(DbcModelTestBase, EcuDataIsCorrect)
{
    model->setDbcConfig(DbcExamples::motorController());

    // Navigate to ECU
    QModelIndex idx = model->index(1, 0, QModelIndex());  // Row 1 is usually the first ECU
    ASSERT_TRUE(idx.isValid());

    // 1. Check Custom Roles
    EXPECT_EQ(getVal(idx, DbcFile::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Ecu);
    EXPECT_EQ(getVal(idx, DbcFile::Role_EcuTotalSignals).toInt(), 3);
    // Child count should be number of messages (MotorController sends 1 message in this example)
    EXPECT_EQ(getVal(idx, DbcFile::Role_ChildCount).toInt(), 1);

    // 2. Check DisplayRole for ALL Columns
    // Col 0: Name
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::EcuName), Qt::DisplayRole)
            .toString(),
        "MotorController");

    // Col 1: Total Signals (Data comes from item->data(1))
    EXPECT_EQ(model
                  ->data(idx.siblingAtColumn(DbcFile::Constants::Columns::EcuTotalSignals),
                         Qt::DisplayRole)
                  .toInt(),
              3);

    // --- Negative Path ---
    assertInvalidRoles(idx, messageRoles);
    assertInvalidRoles(idx, signalRoles);
    EXPECT_FALSE(getVal(idx, DbcFile::Role_Id).isValid());
}

TEST_F(DbcModelTestBase, MessageDataIsCorrect)
{
    model->setDbcConfig(DbcExamples::simple());

    QModelIndex ecu = model->index(1, 0, QModelIndex());
    QModelIndex idx = model->index(0, 0, ecu);  // The message
    ASSERT_TRUE(idx.isValid());

    // 1. Check Custom Roles
    EXPECT_EQ(getVal(idx, DbcFile::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Message);
    EXPECT_EQ(getVal(idx, DbcFile::Role_Id).toUInt(), 0x123);
    EXPECT_EQ(getVal(idx, DbcFile::Role_Dlc).toInt(), 8);
    EXPECT_EQ(getVal(idx, DbcFile::Role_Sender).toString(), "TestNode");
    EXPECT_EQ(getVal(idx, DbcFile::Role_ChildCount).toInt(), 1);  // 1 Signal

    // 2. Check DisplayRole for ALL Columns
    // Col 0: Name
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::MsgName), Qt::DisplayRole)
            .toString(),
        "TestMessage");
    // Col 1: ID
    EXPECT_EQ(model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::MsgId), Qt::DisplayRole)
                  .toUInt(),
              0x123);
    // Col 2: DLC
    EXPECT_EQ(model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::MsgDlc), Qt::DisplayRole)
                  .toInt(),
              8);
    // Col 3: Sender
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::MsgSender), Qt::DisplayRole)
            .toString(),
        "TestNode");
    // Col 4: SigCount (Calculated)
    EXPECT_EQ(
        model->data(idx.siblingAtColumn(DbcFile::Constants::Columns::MsgSigCount), Qt::DisplayRole)
            .toInt(),
        1);

    // --- Negative Path ---
    assertInvalidRoles(idx, ecuRoles);
    assertInvalidRoles(idx, signalRoles);
}

TEST_F(DbcModelTestBase, SignalDataIsCorrect)
{
    model->setDbcConfig(DbcExamples::fullSignalTest());

    // Navigate: Root -> ECU -> Msg -> Sig
    QModelIndex ecu = model->index(1, 0, QModelIndex());
    QModelIndex msg = model->index(0, 0, ecu);
    QModelIndex mainTestSig = model->index(0, 0, msg);
    ASSERT_TRUE(mainTestSig.isValid());

    // 1. Check Custom Roles
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Signal);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Id).toUInt(), 0x100);  // Inherited from Parent

    // 2. Check ALL Columns (DisplayRole) AND corresponding Custom Roles
    // Col 0: Name
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigName),
                         Qt::DisplayRole)
                  .toString(),
              "UnsignedLittleEndSignal");

    // Col 1: Unit
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigUnit),
                         Qt::DisplayRole)
                  .toString(),
              "km/h");
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Unit).toString(), "km/h");

    // Col 2: StartBit
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigStartBit),
                         Qt::DisplayRole)
                  .toInt(),
              7);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_StartBit).toInt(), 7);

    // Col 3: Length
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigLength),
                         Qt::DisplayRole)
                  .toInt(),
              16);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_BitLength).toInt(), 16);

    // Col 4: ByteOrder
    auto endianStr = DbcFile::Constants::SignalsPage::LittleEndIndicator.toStdString();
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigByteOrder),
                         Qt::DisplayRole)
                  .toString()
                  .toStdString(),
              endianStr);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_ByteOrder).toString().toStdString(), endianStr);

    // Col 5: ValueType
    auto typeStr = DbcFile::Constants::SignalsPage::UnsignedIndicator.toStdString();
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigValueType),
                         Qt::DisplayRole)
                  .toString()
                  .toStdString(),
              typeStr);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_ValueType).toString().toStdString(), typeStr);

    // Col 6: Factor (Formatted String "0.5")
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigFactor),
                         Qt::DisplayRole)
                  .toString(),
              "0.5");
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Factor).toString(), "0.5");

    // Col 7: Offset (Formatted String "-10")
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigOffset),
                         Qt::DisplayRole)
                  .toString(),
              "-10");
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Offset).toString(), "-10");

    // Col 8: Min
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigMin),
                         Qt::DisplayRole)
                  .toDouble(),
              -100.0);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Min).toDouble(), -100.0);

    // Col 9: Max
    EXPECT_EQ(model
                  ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigMax),
                         Qt::DisplayRole)
                  .toDouble(),
              100.0);
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Max).toDouble(), 100.0);

    // Col 10: Receivers
    QString recStr =
        model
            ->data(mainTestSig.siblingAtColumn(DbcFile::Constants::Columns::SigReceivers),
                   Qt::DisplayRole)
            .toString();
    EXPECT_EQ(recStr, "ECU1, ECU2, ECU3");
    EXPECT_EQ(getVal(mainTestSig, DbcFile::Role_Receivers).toString(), recStr);

    QModelIndex signedBigEndSig = model->index(1, 0, msg);
    // ByteOrder
    endianStr = DbcFile::Constants::SignalsPage::BigEndIndicator.toStdString();
    EXPECT_EQ(model
                  ->data(signedBigEndSig.siblingAtColumn(DbcFile::Constants::Columns::SigByteOrder),
                         Qt::DisplayRole)
                  .toString()
                  .toStdString(),
              endianStr);
    EXPECT_EQ(getVal(signedBigEndSig, DbcFile::Role_ByteOrder).toString().toStdString(), endianStr);

    // ValueType
    typeStr = DbcFile::Constants::SignalsPage::SignedIndicator.toStdString();
    EXPECT_EQ(model
                  ->data(signedBigEndSig.siblingAtColumn(DbcFile::Constants::Columns::SigValueType),
                         Qt::DisplayRole)
                  .toString()
                  .toStdString(),
              typeStr);
    EXPECT_EQ(getVal(signedBigEndSig, DbcFile::Role_ValueType).toString().toStdString(), typeStr);

    // --- Negative Path ---
    assertInvalidRoles(mainTestSig, ecuRoles);
    assertInvalidRoles(mainTestSig, messageRoles);
}

// ============================================================================
// 3. Edge Cases & Specials
// ============================================================================

TEST_F(DbcModelTestBase, EdgeCase_DecorationDefaults)
{
    model->setDbcConfig(DbcExamples::simple());
    QModelIndex overview = model->index(0, 0, QModelIndex());

    EXPECT_TRUE(model->data(overview, Qt::DecorationRole).isNull());
}

TEST_F(DbcModelTestBase, HandlesEmptyConfig)
{
    model->setDbcConfig(DbcExamples::empty());

    // Only overview row should exist
    EXPECT_EQ(model->rowCount(QModelIndex()), 1);

    QModelIndex overview = model->index(0, 0, QModelIndex());
    EXPECT_TRUE(overview.isValid());

    EXPECT_EQ(getVal(overview, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>(),
              Core::DbcItemType::Overview);

    EXPECT_EQ(getVal(overview).toString(), "empty.dbc");
}

TEST_F(DbcModelTestBase, EdgeCase_OrphanHandling)
{
    model->setDbcConfig(DbcExamples::orphanTest());

    QModelIndex orphanHolderIdx;
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        QModelIndex idx = model->index(i, 0, QModelIndex());
        if (getVal(idx, DbcFile::Role_ItemType).value<Core::DbcItemType>() ==
            Core::DbcItemType::OrphanHolder)
        {
            orphanHolderIdx = idx;
            break;
        }
    }
    ASSERT_TRUE(orphanHolderIdx.isValid());

    EXPECT_EQ(model->rowCount(orphanHolderIdx), 1);
    EXPECT_EQ(getVal(model->index(0, 0, orphanHolderIdx)).toString(), "GhostMsg");

    // --- Negative Path ---
    assertInvalidRoles(orphanHolderIdx, ecuRoles);
    assertInvalidRoles(orphanHolderIdx, messageRoles);
    assertInvalidRoles(orphanHolderIdx, signalRoles);
    EXPECT_FALSE(getVal(orphanHolderIdx, DbcFile::Role_Id).isValid());
}

TEST_F(DbcModelTestBase, EdgeCase_DoesNotCreateOrphanHolderWithoutOrphans)
{
    model->setDbcConfig(DbcExamples::simple());

    for (int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        const QModelIndex idx = model->index(i, 0, QModelIndex());
        const auto type = getVal(idx, DbcFile::Role_ItemType).value<Core::DbcItemType>();
        EXPECT_NE(type, Core::DbcItemType::OrphanHolder);
    }
}

TEST_F(DbcModelTestBase, EdgeCase_UnknownRoles)
{
    model->setDbcConfig(DbcExamples::simple());
    QModelIndex msg = model->index(0, 0, model->index(1, 0, QModelIndex()));

    EXPECT_FALSE(getVal(msg, Qt::UserRole + 9999).isValid());
}

TEST_F(DbcModelTestBase, MultipleLoadConfig_ResetsCleanly)
{
    model->setDbcConfig(DbcExamples::simple());
    int firstCount = model->rowCount(QModelIndex());
    EXPECT_GT(firstCount, 0);

    model->setDbcConfig(DbcExamples::vehicleSensors());
    int secondCount = model->rowCount(QModelIndex());
    EXPECT_GT(secondCount, 0);

    // Ensure model did not accumulate old data
    EXPECT_NE(firstCount, secondCount);
}
TEST_F(DbcModelTestBase, Logic_Linking_IsCaseSensitive)
{
    auto config =
        DbcConfigBuilder()
            .node("MyECU")
            .message(DbcMessageBuilder(1, "SuccessMsg").transmitter("MyECU"))  // Match
            .message(DbcMessageBuilder(2, "FailMsg").transmitter("myecu"))     // Mismatch (Case)
            .build();

    model->setDbcConfig(config);

    // Ecu has to hold SuccessMsg
    QModelIndex ecuIdx = model->index(1, 0, QModelIndex());
    EXPECT_EQ(getVal(ecuIdx).toString().toStdString(), "MyECU");
    EXPECT_EQ(model->rowCount(ecuIdx), 1);
    EXPECT_EQ(getVal(model->index(0, 0, ecuIdx)).toString().toStdString(), "SuccessMsg");

    // Orphan Holder has to hold FailMsg
    QModelIndex orphanIdx;
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i)
    {
        QModelIndex idx = model->index(i, 0, QModelIndex());
        auto type = getVal(idx, DbcFile::DbcRoles::Role_ItemType).value<Core::DbcItemType>();
        if (type == Core::DbcItemType::OrphanHolder) orphanIdx = idx;
    }

    ASSERT_TRUE(orphanIdx.isValid());
    EXPECT_EQ(getVal(model->index(0, 0, orphanIdx)).toString().toStdString(), "FailMsg");
}
