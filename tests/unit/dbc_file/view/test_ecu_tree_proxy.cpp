#include <gtest/gtest.h>

#include <QStandardItem>
#include <QStandardItemModel>

#include "core/enum/dbc_itemtype.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/view/proxies/ecu_tree_proxy.hpp"

using namespace DbcFile;

class TestableEcuTreeProxy : public EcuTreeProxy
{
   public:
    explicit TestableEcuTreeProxy(QObject* parent = nullptr) : EcuTreeProxy(parent) {}

    bool publicFilterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        return filterAcceptsRow(sourceRow, sourceParent);
    }
};

class EcuTreeProxyTest : public ::testing::Test
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
        // --- 1. OVERVIEW (should always be hidden) ---
        dummyModel.appendRow(createItem("Overview", Core::DbcItemType::Overview));

        // --- 2. ENGINE ECU (Active - has children) ---
        auto* engineEcu = createItem("EngineECU", Core::DbcItemType::Ecu);
        engineEcu->setData(1, DbcFile::DbcRoles::Role_ChildCount);
        dummyModel.appendRow(engineEcu);

        // Message: SpeedMsg
        auto* speedMsg = createItem("SpeedMsg", Core::DbcItemType::Message);
        engineEcu->appendRow(speedMsg);

        // Signal: VehicleSpeed (leaf - should be hidden)
        auto* speedSig = createItem("VehicleSpeed", Core::DbcItemType::Signal);
        speedMsg->appendRow(speedSig);

        // --- 3. GATEWAY ECU (Passive - has no children) ---
        auto* gatewayEcu = createItem("GatewayECU", Core::DbcItemType::Ecu);
        gatewayEcu->setData(0, DbcFile::DbcRoles::Role_ChildCount);
        dummyModel.appendRow(gatewayEcu);

        // --- 4. ORPHAN HOLDER (should always be hidden) ---
        auto* orphanHolder = createItem("Orphan Messages", Core::DbcItemType::OrphanHolder);
        dummyModel.appendRow(orphanHolder);
    }
};

// ============================================================================
// TESTS: Structural Filtering (Overview & Orphans)
// ============================================================================

TEST_F(EcuTreeProxyTest, HidesSpecialNodes)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 2);

    QString name0 = proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString();
    QString name1 = proxy.data(proxy.index(1, 0, QModelIndex()), Qt::DisplayRole).toString();

    EXPECT_EQ(name0, "EngineECU");
    EXPECT_EQ(name1, "GatewayECU");
}

// ============================================================================
// TESTS: Category Filtering (Active vs All)
// ===========================================================================

TEST_F(EcuTreeProxyTest, Filter_SendingOnly)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    // Set filter to Active/Sending only
    proxy.setFilterCategory(Constants::EcusPage::FilterActiveIndex);

    // GateWayECU has to vanish
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);

    QString name = proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString();
    EXPECT_EQ(name, "EngineECU");
}

TEST_F(EcuTreeProxyTest, Filter_All)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    // Filter first
    proxy.setFilterCategory(Constants::EcusPage::FilterActiveIndex);
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 1);

    // Reset filter
    proxy.setFilterCategory(0);

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 2);
}

TEST_F(EcuTreeProxyTest, Filter_Category_ActiveOnly_KeepsActiveEcus)
{
    TestableEcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    proxy.setFilterCategory(Constants::EcusPage::FilterActiveIndex);

    int sourceRowEngine = 1;

    bool accepted = proxy.publicFilterAcceptsRow(sourceRowEngine, QModelIndex());

    EXPECT_TRUE(accepted);
}

// ============================================================================
// TESTS: Text Search Filtering
// ============================================================================

TEST_F(EcuTreeProxyTest, Filter_Text_MatchesCaseInsensitive)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    proxy.setSearchText("gateway");

    ASSERT_EQ(proxy.rowCount(QModelIndex()), 1);
    QString name = proxy.data(proxy.index(0, 0, QModelIndex()), Qt::DisplayRole).toString();
    EXPECT_EQ(name, "GatewayECU");
}

TEST_F(EcuTreeProxyTest, Filter_Text_NoMatchHidesAll)
{
    TestableEcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    proxy.setSearchText("NonExistentThing");

    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);

    int sourceRowEngine = 1;
    bool accepted = proxy.publicFilterAcceptsRow(sourceRowEngine, QModelIndex());
    EXPECT_FALSE(accepted);
}

// ============================================================================
// TESTS: Tree Structure Logic (Leaves)
// ============================================================================

TEST_F(EcuTreeProxyTest, Logic_HasChildren_BlocksSignals)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    // EngineECU (Row 0 in proxy)
    QModelIndex engineIdx = proxy.index(0, 0, QModelIndex());
    ASSERT_TRUE(engineIdx.isValid());

    // ECU has children -> expect true
    EXPECT_TRUE(proxy.hasChildren(engineIdx));

    QModelIndex msgIdx = proxy.index(0, 0, engineIdx);
    ASSERT_TRUE(msgIdx.isValid());
    EXPECT_EQ(proxy.data(msgIdx, Qt::DisplayRole).toString(), "SpeedMsg");

    // signals have to be blocked in proxy
    EXPECT_FALSE(proxy.hasChildren(msgIdx));
    EXPECT_EQ(proxy.rowCount(msgIdx), 0);

    // msg without signals -> default behaviour
    QModelIndex gatewayIdx = proxy.index(1, 0, QModelIndex());
    EXPECT_FALSE(proxy.hasChildren(gatewayIdx));
}

// ============================================================================
// TESTS: Columns & Robustness
// ============================================================================

TEST_F(EcuTreeProxyTest, Logic_ColumnCount_IsAlwaysOne)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    // Top Level
    EXPECT_EQ(proxy.columnCount(QModelIndex()), 1);

    // Child Level
    QModelIndex engineIdx = proxy.index(0, 0, QModelIndex());
    EXPECT_EQ(proxy.columnCount(engineIdx), 1);
}

TEST_F(EcuTreeProxyTest, Robustness_InvalidIndices)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    QModelIndex invalid;

    EXPECT_TRUE(proxy.hasChildren(invalid));

    EXPECT_EQ(proxy.columnCount(invalid), 1);
}

TEST_F(EcuTreeProxyTest, Robustness_HandlesNullModel)
{
    EcuTreeProxy proxy;
    proxy.setSourceModel(nullptr);
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);
}

TEST_F(EcuTreeProxyTest, Robustness_HandlesEmptyModel)
{
    EcuTreeProxy proxy;
    QStandardItemModel emptyModel;
    proxy.setSourceModel(&emptyModel);
    EXPECT_EQ(proxy.rowCount(QModelIndex()), 0);
}

TEST_F(EcuTreeProxyTest, Robustness_FilterAcceptsRow_HandlesInvalidIndex)
{
    TestableEcuTreeProxy proxy;
    proxy.setSourceModel(&dummyModel);

    bool accepted = proxy.publicFilterAcceptsRow(999, QModelIndex());  // Root parent

    EXPECT_FALSE(accepted);
}
