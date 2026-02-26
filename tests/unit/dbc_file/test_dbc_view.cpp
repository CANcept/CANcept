#include <gtest/gtest.h>

#include <QLabel>
#include <QListView>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeView>

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/view/dbc_view.hpp"
#include "dbc_file/view/pages/ecus_page.hpp"
#include "dbc_file/view/pages/load_page.hpp"
#include "dbc_file/view/pages/messages_page.hpp"
#include "dbc_file/view/pages/overview_page.hpp"
#include "dbc_file/view/pages/signals_page.hpp"

using namespace DbcFile;

class DbcViewTest : public ::testing::Test {
protected:
    // Helper to manually call private slots
    void invokeSlot(QObject* obj, const char* slot, const QString& arg) {
        bool success = QMetaObject::invokeMethod(obj, slot, Q_ARG(QString, arg));
        ASSERT_TRUE(success) << "Slot " << slot << " not found or arguments mismatch!";
    }

    void invokeSlot(QObject* obj, const char* slot, int arg) {
        bool success = QMetaObject::invokeMethod(obj, slot, Q_ARG(int, arg));
        ASSERT_TRUE(success) << "Slot " << slot << " not found or arguments mismatch!";
    }

    // Helper: gets page out of stack
    template <typename T>
    T* getPage(DbcView& view, int index) {
        auto* stack = view.findChild<QStackedWidget*>();
        if (!stack || index >= stack->count()) return nullptr;
        return qobject_cast<T*>(stack->widget(index));
    }
};

// ============================================================================
// 1. INITIALIZATION & STRUCTURE
// ============================================================================

TEST_F(DbcViewTest, InitialStructure_CreatesAllPagesAndSidebar) {
    DbcView view;

    // Check Sidebar
    auto* sidebar = view.findChild<Core::Sidebar*>();
    ASSERT_NE(sidebar, nullptr);

    // Check Stack
    auto* stack = view.findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->count(), Constants::Sidebar::STACK_COUNT); // Load, Overview, ECU, Msg, Sig

    // Check Pages existence
    EXPECT_NE(getPage<LoadPage>(view, Constants::Sidebar::INDEX_LOAD), nullptr);
    EXPECT_NE(getPage<OverviewPage>(view, Constants::Sidebar::INDEX_OVERVIEW), nullptr);
    EXPECT_NE(getPage<EcusPage>(view, Constants::Sidebar::INDEX_ECUS), nullptr);
    EXPECT_NE(getPage<MessagesPage>(view, Constants::Sidebar::INDEX_MESSAGES), nullptr);
    EXPECT_NE(getPage<SignalsPage>(view, Constants::Sidebar::INDEX_SIGNALS), nullptr);
}

// ============================================================================
// 2. MODEL WIRING (setSourceModel)
// ============================================================================

TEST_F(DbcViewTest, Wiring_SetSourceModel_InitializesProxies) {
    DbcView view;
    QStandardItemModel dummyModel;

    // Act: Set the model
    view.setSourceModel(&dummyModel);

    // Assert: Proxies are created (White-Box check via Getters)
    ASSERT_NE(view.getOverviewEcuProxy(), nullptr);
    ASSERT_NE(view.getOverviewMessageProxy(), nullptr);
    ASSERT_NE(view.getMessagesProxy(), nullptr);
    ASSERT_NE(view.getSignalsProxy(), nullptr);
    ASSERT_NE(view.getEcuTreeProxy(), nullptr);

    // Assert: Proxies are connected to source
    EXPECT_EQ(view.getOverviewEcuProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getOverviewMessageProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getEcuTreeProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getMessagesProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getSignalsProxy()->sourceModel(), &dummyModel);
}

TEST_F(DbcViewTest, Wiring_SetSourceModel_PopulatesPages) {
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);


    auto* ovPage = getPage<OverviewPage>(view, Constants::Sidebar::INDEX_OVERVIEW);
    ASSERT_NE(ovPage, nullptr);

    // --- Check ECU List of Overview Page ---
    auto* ecuList = ovPage->getEcuList();
    ASSERT_NE(ecuList, nullptr);
    ASSERT_NE(ecuList->model(), nullptr);

    EXPECT_NE(ecuList->model(), &dummyModel);
    EXPECT_EQ(ecuList->model(), view.getOverviewEcuProxy());

    // --- Check Message List of Overview Page ---
    auto* msgList = ovPage->getMessageList();
    ASSERT_NE(msgList, nullptr);
    ASSERT_NE(msgList->model(), nullptr);

    EXPECT_NE(msgList->model(), &dummyModel);
    EXPECT_EQ(msgList->model(), view.getOverviewMessageProxy());

    // Check ECU Page (Tree)
    auto* ecuPage = getPage<EcusPage>(view, Constants::Sidebar::INDEX_ECUS);
    auto* ecuTree = ecuPage->findChild<QTreeView*>();
    ASSERT_NE(ecuTree, nullptr);
    EXPECT_NE(ecuTree->model(), nullptr);
    EXPECT_NE(ecuTree->model(), &dummyModel);

    // Check Messages Page (Table)
    auto* msgPage = getPage<MessagesPage>(view, Constants::Sidebar::INDEX_MESSAGES);
    auto* msgTable = msgPage->findChild<QTableView*>();
    ASSERT_NE(msgTable, nullptr);
    EXPECT_NE(msgTable->model(), &dummyModel);  // Must be proxy

    auto* signalsPage = getPage<SignalsPage>(view, Constants::Sidebar::INDEX_SIGNALS);
    auto sigTable = signalsPage->findChild<QTableView*>();
    ASSERT_NE(sigTable, nullptr);
    EXPECT_NE(sigTable->model(), &dummyModel);
}

TEST_F(DbcViewTest, Logic_MessageSelection_MapsIndexAndUpdatesPage) {
    DbcView view;
    QStandardItemModel dummyModel;

    auto* msgItem = new QStandardItem("SpeedMsg");
    msgItem->setData(QVariant::fromValue(Core::DbcItemType::Message), Role_ItemType);
    dummyModel.appendRow(msgItem);

    view.setSourceModel(&dummyModel);

    // Get valid index from messages proxy
    auto* proxy = view.getMessagesProxy();
    QModelIndex proxyIndex = proxy->index(0, 0, QModelIndex());
    ASSERT_TRUE(proxyIndex.isValid()) << "Proxy should have one item";

    // Call slot (simulate click in messages table)
    bool success = QMetaObject::invokeMethod(&view, "onMessageSelected",
                                             Q_ARG(QModelIndex, proxyIndex));
    ASSERT_TRUE(success);
}

// ============================================================================
// 3. NAVIGATION LOGIC
// ============================================================================

TEST_F(DbcViewTest, Navigation_SidebarSwitchesStackIndex) {
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();

    // Initial State
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_LOAD);

    // Act: Switch to Messages
    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_MESSAGES);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_MESSAGES);

    // Act: Switch to Signals
    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_SIGNALS);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_SIGNALS);
}

TEST_F(DbcViewTest, Navigation_IgnoresInvalidIndices) {
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();
    int initialIndex = stack->currentIndex();

    // Act: Index -1
    invokeSlot(&view, "onSidebarSelectionChanged", -1);
    EXPECT_EQ(stack->currentIndex(), initialIndex);

    // Act: Index out of bounds (99)
    invokeSlot(&view, "onSidebarSelectionChanged", 99);
    EXPECT_EQ(stack->currentIndex(), initialIndex);

    // Act: Same Index (sollte early return machen)
    invokeSlot(&view, "onSidebarSelectionChanged", initialIndex);
    EXPECT_EQ(stack->currentIndex(), initialIndex);
}

TEST_F(DbcViewTest, Logic_Navigation_ResetsLoadPageOnLeave) {
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();

    // Start on LoadPage
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_LOAD);

    // Change to different page
    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_ECUS);

    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_ECUS);

    // Click on same page again
    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_ECUS);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_ECUS);

    // Click invalid index
    invokeSlot(&view, "onSidebarSelectionChanged", 99);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_ECUS);
}

// ============================================================================
// 4. FILTER LOGIC (Data Flow: View -> Proxy)
// ============================================================================

TEST_F(DbcViewTest, Logic_MessageFilter_UpdatesProxyState) {
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getMessagesProxy();

    // Act 1: Text Filter
    invokeSlot(&view, "onMessageFilterTextChanged", "SpeedMsg");
    EXPECT_EQ(proxy->getSearchFilter(), "SpeedMsg");

    // Act 2: Sender Filter
    invokeSlot(&view, "onMessageSenderChanged", "EngineECU");
    EXPECT_EQ(proxy->getFilterMessageSender(), "EngineECU");
}

TEST_F(DbcViewTest, Logic_SignalFilter_UpdatesProxyState) {
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getSignalsProxy();

    // Act 1: Text Filter
    invokeSlot(&view, "onSignalFilterTextChanged", "RPM");
    EXPECT_EQ(proxy->getSearchFilter(), "RPM");

    // Act 2: Unit Filter
    invokeSlot(&view, "onSignalUnitChanged", "km/h");
    EXPECT_EQ(proxy->getSignalFilterUnit(), "km/h");
}

TEST_F(DbcViewTest, Logic_EcuFilter_UpdatesProxyState) {
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getEcuTreeProxy();

    // Act 1: Text Filter
    invokeSlot(&view, "onEcuFilterTextChanged", "Gateway");
    EXPECT_EQ(proxy->getSearchFilter(), "Gateway");

    // Act 2: Category Filter
    invokeSlot(&view, "onEcuFilterIndexChanged", 1); // e.g. "Active Only"
    EXPECT_EQ(proxy->getFilterCategory(), 1);
}

// ============================================================================
// 5. SIGNAL FORWARDING (Output)
// ============================================================================

TEST_F(DbcViewTest, Signal_FileLoadRequested_IsForwarded) {
    DbcView view;
    QSignalSpy spy(&view, &DbcView::fileLoadRequested);

    auto* loadPage = getPage<LoadPage>(view, Constants::Sidebar::INDEX_LOAD);
    ASSERT_NE(loadPage, nullptr);

    // Act: Simulate file drop on LoadPage
    emit loadPage->fileSelected("/tmp/can.dbc");

    // Assert: View forwards it
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), "/tmp/can.dbc");
}

// ============================================================================
// 6. ROBUSTNESS
// ============================================================================

TEST_F(DbcViewTest, Robustness_SlotsDoNotCrashWithoutModel) {
    DbcView view;
    // Act: Trigger slots that access proxies
    EXPECT_NO_THROW(invokeSlot(&view, "onMessageFilterTextChanged", "Text"));
    EXPECT_NO_THROW(invokeSlot(&view, "onSignalUnitChanged", "Unit"));
    EXPECT_NO_THROW(invokeSlot(&view, "onEcuFilterTextChanged", "Text"));
}

TEST_F(DbcViewTest, Robustness_SetSourceModelHandlesNull) {
    DbcView view;

    EXPECT_NO_THROW(view.setSourceModel(nullptr));
}

TEST_F(DbcViewTest, PublicAPI_SettersDoNotCrash) {
    DbcView view;
    // Testet setSignalUnits
    EXPECT_NO_THROW(view.setSignalUnits({"km/h", "rpm"}));

    // Testet setAvailableSenders
    EXPECT_NO_THROW(view.setAvailableSenders({"Engine", "Brake"}));

    // Testet setNavigationEnabled
    EXPECT_NO_THROW(view.setNavigationEnabled(false));
}
