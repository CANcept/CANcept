#include <QStandardItemModel>

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/view/proxies/flat_list_proxy.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/dbc_config_builder.hpp"
class FlatListProxyTest : public ::testing::Test
{
   protected:
    QStandardItemModel dummyModel;

    static QStandardItem* createItem(const QString& name, Core::DbcItemType type)
    {
        auto* item = new QStandardItem(name);
        item->setData(QVariant::fromValue(type), DbcFile::DbcRoles::Role_ItemType);
        return item;
    }
    void SetUp() override
    {
        // --- 1. OVERVIEW
        dummyModel.appendRow(createItem("Overview", Core::DbcItemType::Overview));

        // --- 2. ENGINE ECU (Active) ---
        auto* engineEcu = createItem("EngineECU", Core::DbcItemType::Ecu);
        engineEcu->setData(1, DbcFile::DbcRoles::Role_ChildCount);
        dummyModel.appendRow(engineEcu);

        // Message: SpeedMsg
        auto* speedMsg = createItem("SpeedMsg", Core::DbcItemType::Message);
        speedMsg->setData("EngineECU", DbcFile::DbcRoles::Role_Sender);
        engineEcu->appendRow(speedMsg);

        // Signal: VehicleSpeed
        auto* speedSig = createItem("VehicleSpeed", Core::DbcItemType::Signal);
        speedSig->setData("km/h", DbcFile::DbcRoles::Role_Unit);
        speedMsg->appendRow(speedSig);

        // Signal: RPM
        auto* rpmSig = createItem("EngineRPM", Core::DbcItemType::Signal);
        rpmSig->setData("rpm", DbcFile::DbcRoles::Role_Unit);
        speedMsg->appendRow(rpmSig);

        // --- 3. BRAKE ECU (Active) ---
        auto* brakeEcu = createItem("BrakeECU", Core::DbcItemType::Ecu);
        brakeEcu->setData(1, DbcFile::DbcRoles::Role_ChildCount);
        dummyModel.appendRow(brakeEcu);

        // Message: BrakeMsg
        auto* brakeMsg = createItem("BrakeMsg", Core::DbcItemType::Message);
        brakeMsg->setData("BrakeECU", DbcFile::DbcRoles::Role_Sender);
        brakeEcu->appendRow(brakeMsg);

        // Signal: BrakeTemp
        auto* tempSig = createItem("BrakeTemp", Core::DbcItemType::Signal);
        tempSig->setData("C", DbcFile::DbcRoles::Role_Unit);
        brakeMsg->appendRow(tempSig);

        // --- 4. GATEWAY ECU (Passive / Empty) ---
        auto* gatewayEcu = createItem("GatewayECU", Core::DbcItemType::Ecu);
        gatewayEcu->setData(0, DbcFile::DbcRoles::Role_ChildCount);
        dummyModel.appendRow(gatewayEcu);

        // --- 5. ORPHAN HOLDER ---
        auto* orphanHolder = createItem("Orphan Messages", Core::DbcItemType::OrphanHolder);
        dummyModel.appendRow(orphanHolder);

        // Message: UnknownMsg
        auto* unknownMsg = createItem("UnknownMsg", Core::DbcItemType::Message);
        unknownMsg->setData("Unknown", DbcFile::DbcRoles::Role_Sender);
        orphanHolder->appendRow(unknownMsg);

        // Signal: GhostSig
        auto* ghostSig = createItem("GhostSig", Core::DbcItemType::Signal);
        ghostSig->setData("", DbcFile::DbcRoles::Role_Unit);
        unknownMsg->appendRow(ghostSig);
    }
};

// ============================================================================
// TESTS: FlatListProxy (Basic Functionality by Type)
// ============================================================================

TEST_F(FlatListProxyTest, FlatList_ECUs_FlattensHierarchy)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 3);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "EngineECU");
    EXPECT_EQ(proxy.data(proxy.index(1, 0, QModelIndex()), Qt::DisplayRole).toString(), "BrakeECU");
    EXPECT_EQ(proxy.data(proxy.index(2, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "GatewayECU");
}

TEST_F(FlatListProxyTest, FlatList_Messages_IncludesOrphans)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 3);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "SpeedMsg");
    EXPECT_EQ(proxy.data(proxy.index(1, 0, QModelIndex()), Qt::DisplayRole).toString(), "BrakeMsg");
    EXPECT_EQ(proxy.data(proxy.index(2, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "UnknownMsg");
}

TEST_F(FlatListProxyTest, FlatList_Signals_FlattensDeepHierarchy)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 4);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "VehicleSpeed");
    EXPECT_EQ(proxy.data(proxy.index(1, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "EngineRPM");
    EXPECT_EQ(proxy.data(proxy.index(2, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "BrakeTemp");
    EXPECT_EQ(proxy.data(proxy.index(3, 0, QModelIndex()), Qt::DisplayRole).toString(), "GhostSig");
}

TEST_F(FlatListProxyTest, FlatList_TargetOverview_FindsSingleItem)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Overview);
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);

    QModelIndex idx = proxy.index(0, 0, QModelIndex());
    EXPECT_EQ(proxy.data(idx, Qt::DisplayRole).toString(), "Overview");
}

TEST_F(FlatListProxyTest, FlatList_TargetOrphanHolder_FindsContainer)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::OrphanHolder);
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);

    QModelIndex idx = proxy.index(0, 0, QModelIndex());
    QString name = proxy.data(idx, Qt::DisplayRole).toString();

    EXPECT_EQ(name, "Orphan Messages");
}

TEST_F(FlatListProxyTest, Logic_ScanNode_StopsRecursionAtTarget)
{
    // Setup: Create a nested structure that is technically possible in QStandardItemModel
    // Root -> ECU1 -> ChildItem(Type=ECU)
    // If logic is correct, ChildItem should NOT be found because recursion stops at ECU1.

    QStandardItemModel nestedModel;
    auto* ecu1 = createItem("ParentECU", Core::DbcItemType::Ecu);
    auto* ecu2 = createItem("ChildECU", Core::DbcItemType::Ecu);  // Nested ECU
    ecu1->appendRow(ecu2);
    nestedModel.appendRow(ecu1);

    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);
    proxy.setSourceModel(&nestedModel);

    // Assert: Should only find "ParentECU".
    // If 'stopHere' logic was broken, it might find 2 items.
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "ParentECU");
}

// ============================================================================
// TESTS: Filtering Logic
// ============================================================================

TEST_F(FlatListProxyTest, FlatList_Messages_FilterBySender)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);
    proxy.setSourceModel(&dummyModel);

    // Act: Filter for "BrakeECU"
    proxy.setFilterMessageSender("BrakeECU");

    // Assert: only "BrakeMsg"
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "BrakeMsg");

    // Check same sender again
    proxy.setFilterMessageSender("BrakeECU");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "BrakeMsg");
}

TEST_F(FlatListProxyTest, FlatList_Messages_FilterByText)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);
    proxy.setSourceModel(&dummyModel);

    // Act: Search for "Speed"
    proxy.setSearchFilter("Speed");

    // Assert: only "SpeedMsg"
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "SpeedMsg");

    proxy.setSearchFilter("Speed");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "SpeedMsg");
}

TEST_F(FlatListProxyTest, FlatList_Messages_CombinesSenderAndTextFilters)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);
    proxy.setSourceModel(&dummyModel);

    proxy.setFilterMessageSender("EngineECU");
    proxy.setSearchFilter("Brake");
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);

    proxy.setSearchFilter("Speed");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(), "SpeedMsg");
}

TEST_F(FlatListProxyTest, FlatList_Signals_FilterByUnit)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    // Act: Filter for "km/h" signals
    proxy.setSignalFilterUnit("km/h");

    // Assert: only "VehicleSpeed"
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    QModelIndex idx = proxy.index(0, 0, QModelIndex());
    EXPECT_EQ(proxy.data(idx, Qt::DisplayRole).toString(), "VehicleSpeed");

    proxy.setSignalFilterUnit("km/h");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(idx, Qt::DisplayRole).toString(), "VehicleSpeed");
}

TEST_F(FlatListProxyTest, FlatList_Signals_FilterByText)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    // Act: Search for "RPM"
    proxy.setSearchFilter("RPM");

    // Assert: only "EngineRPM"
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "EngineRPM");

    proxy.setSearchFilter("RPM");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "EngineRPM");
}

TEST_F(FlatListProxyTest, FlatList_Signals_CombinesUnitAndTextFilters)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    proxy.setSignalFilterUnit("km/h");
    proxy.setSearchFilter("rpm");
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);

    proxy.setSearchFilter("speed");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    EXPECT_EQ(proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString(),
              "VehicleSpeed");
}

TEST_F(FlatListProxyTest, FlatList_SearchYieldsNoResults)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    proxy.setSearchFilter("NonExistentXYZ");

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);

    EXPECT_FALSE(proxy.index(0, 0, QModelIndex()).isValid());
}

TEST_F(FlatListProxyTest, FlatList_ClearingSearchFilterRestoresAllResults)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    ASSERT_EQ(proxy.rowCount(QModelIndex()), 4);

    proxy.setSearchFilter("RPM");
    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);

    proxy.setSearchFilter("");
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 4);
}

TEST_F(FlatListProxyTest, FlatList_IgnoresSenderFilterOnSignals)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    int countBefore = proxy.rowCount(QModelIndex());

    // Set sender filter (intended only for messages)
    proxy.setFilterMessageSender("EngineECU");

    // Filter should be ignored
    int countAfter = proxy.rowCount(QModelIndex());
    EXPECT_EQ(countBefore, countAfter);
}

// ============================================================================
// TESTS: Robustness & Structure
// ============================================================================

TEST_F(FlatListProxyTest, FlatList_HandlesNullModel)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);

    proxy.setSourceModel(nullptr);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);
}

TEST_F(FlatListProxyTest, FlatList_HandlesEmptyModel)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Message);

    QStandardItemModel emptyModel;
    proxy.setSourceModel(&emptyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);
}

TEST_F(FlatListProxyTest, FlatList_UpdatesOnModelReset)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);
    QStandardItemModel dynamicModel;
    proxy.setSourceModel(&dynamicModel);

    // Initially empty
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);

    // Add item and manually reset model
    dynamicModel.appendRow(createItem("NewECU", Core::DbcItemType::Ecu));
    proxy.setSourceModel(nullptr);
    proxy.setSourceModel(&dynamicModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);
}

TEST_F(FlatListProxyTest, ReactsToRealModelResetSignal)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);
    proxy.setSourceModel(&dummyModel);

    // Initial count
    int initialCount = proxy.rowCount(QModelIndex());
    EXPECT_EQ(initialCount, 3);

    // Act: Clear the source model (triggers beginResetModel/endResetModel internally)
    dummyModel.clear();

    // Assert: Proxy should have updated automatically via signal connection
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);
}

// ============================================================================
// TESTS: API Compliance (Mapping, Headers, Parents)
// ============================================================================

TEST_F(FlatListProxyTest, Api_Mapping_BiDirectional)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    // Get index of first proxy signal
    QModelIndex proxyIndex = proxy.index(0, 0, QModelIndex());
    ASSERT_TRUE(proxyIndex.isValid());

    // Map to source
    QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);
    ASSERT_TRUE(sourceIndex.isValid());

    // Check: right item in sourcemodel?
    QString sourceName = dummyModel.data(sourceIndex, Qt::DisplayRole).toString();
    EXPECT_EQ(sourceName, "VehicleSpeed");

    // Map back to proxy
    QModelIndex backToProxyIndex = proxy.mapFromSource(sourceIndex);
    ASSERT_TRUE(backToProxyIndex.isValid());

    EXPECT_EQ(proxyIndex, backToProxyIndex);
    EXPECT_EQ(proxyIndex.row(), backToProxyIndex.row());
}

TEST_F(FlatListProxyTest, Api_Mapping_InvalidIndices)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    // mapToSource with invalid index
    QModelIndex invalid = QModelIndex();
    EXPECT_FALSE(proxy.mapToSource(invalid).isValid());

    // mapFromSource with invalid index
    EXPECT_FALSE(proxy.mapFromSource(invalid).isValid());

    // Index out of bounds
    QModelIndex outOfBounds = proxy.index(999, 0, QModelIndex());
    EXPECT_FALSE(outOfBounds.isValid());

    QModelIndex colOutOfBounds = proxy.index(0, 99, QModelIndex());
    EXPECT_FALSE(colOutOfBounds.isValid());
}

TEST_F(FlatListProxyTest, Api_Mapping_Hidden_Item)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    // Search source item "VehicleSpeed" manually
    QModelIndex ecuIdx = dummyModel.index(1, 0);
    QModelIndex msgIdx = dummyModel.index(0, 0, ecuIdx);
    QModelIndex sourceSpeedIdx = dummyModel.index(0, 0, msgIdx);

    ASSERT_EQ(dummyModel.data(sourceSpeedIdx, Qt::DisplayRole).toString(), "VehicleSpeed");

    // Set filter to hide VehicleSpeed
    proxy.setSearchFilter("EngineRPM");

    // Map hidden source item
    QModelIndex proxyIdx = proxy.mapFromSource(sourceSpeedIdx);

    EXPECT_FALSE(proxyIdx.isValid());
}

TEST_F(FlatListProxyTest, FlatList_Structure_IsTrulyFlat)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Signal);
    proxy.setSourceModel(&dummyModel);

    QModelIndex idx = proxy.index(0, 0, QModelIndex());

    // Item cannot have a parent (flat list)
    QModelIndex parentIdx = proxy.parent(idx);
    EXPECT_FALSE(parentIdx.isValid());

    // Check correct columncount
    EXPECT_EQ(proxy.columnCount(QModelIndex()), DbcFile::Constants::Columns::SignalColumnCount);
}

TEST_F(FlatListProxyTest, Api_Headers)
{
    // Messages
    DbcFile::FlatListProxy msgProxy(Core::DbcItemType::Message);

    QVariant idHeader =
        msgProxy.headerData(DbcFile::Constants::Columns::MsgId, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_EQ(idHeader.toString(), DbcFile::Constants::Headers::MsgId);
    QVariant invalidMsgHeader = msgProxy.headerData(100, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_FALSE(invalidMsgHeader.isValid());

    // Signals
    DbcFile::FlatListProxy sigProxy(Core::DbcItemType::Signal);

    QVariant unitHeader =
        sigProxy.headerData(DbcFile::Constants::Columns::SigUnit, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_EQ(unitHeader.toString(), DbcFile::Constants::Headers::SigUnit);
    QVariant invalidSigHeader = msgProxy.headerData(100, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_FALSE(invalidSigHeader.isValid());

    // 3. Vertical Header (Standard Qt behavior)
    QVariant vertHeader = sigProxy.headerData(0, Qt::Vertical, Qt::DisplayRole);
    EXPECT_TRUE(vertHeader.isValid());
}

TEST_F(FlatListProxyTest, Api_HeaderData_ReturnsEmptyForUnhandledTypes)
{
    // TargetType is ECU (neither Message nor Signal)
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);

    QVariant result = proxy.headerData(0, Qt::Horizontal, Qt::DisplayRole);

    EXPECT_FALSE(result.isValid());
}

TEST_F(FlatListProxyTest, Api_ColumnCount_DelegatesToSourceForEcus)
{
    DbcFile::FlatListProxy proxy(Core::DbcItemType::Ecu);

    // Case 1: No Source Model -> returns 0
    EXPECT_EQ(proxy.columnCount(QModelIndex()), 0);

    // Case 2: With Source Model -> delegates to dummyModel.columnCount()
    proxy.setSourceModel(&dummyModel);

    // QStandardItemModel usually has 1 column by default unless changed
    EXPECT_EQ(proxy.columnCount(QModelIndex()), dummyModel.columnCount());
}
