#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QStandardItemModel>

#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/view/dbc_view.hpp"
#include "dbc_file/view/pages/ecus_page.hpp"
#include "dbc_file/view/pages/load_page.hpp"
#include "dbc_file/view/pages/messages_page.hpp"
#include "dbc_file/view/pages/overview_page.hpp"
#include "dbc_file/view/pages/signals_page.hpp"

using namespace DbcFile;

class DbcViewLogicTest : public ::testing::Test
{
   protected:
    static void invokeSlot(QObject* obj, const char* slot, const QString& arg)
    {
        bool success = QMetaObject::invokeMethod(obj, slot, Q_ARG(QString, arg));
        ASSERT_TRUE(success) << "Slot " << slot << " not found or arguments mismatch!";
    }

    static void invokeSlot(QObject* obj, const char* slot, int arg)
    {
        bool success = QMetaObject::invokeMethod(obj, slot, Q_ARG(int, arg));
        ASSERT_TRUE(success) << "Slot " << slot << " not found or arguments mismatch!";
    }

    template <typename T>
    T* getPage(DbcView& view, int index)
    {
        auto* stack = view.findChild<QStackedWidget*>();
        if (!stack || index >= stack->count()) return nullptr;
        return qobject_cast<T*>(stack->widget(index));
    }
};

// ============================================================================
// 1. INITIALIZATION & STRUCTURE
// ============================================================================
TEST_F(DbcViewLogicTest, InitialStructure_PagesExist)
{
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);

    EXPECT_EQ(stack->count(), Constants::Sidebar::STACK_COUNT);

    EXPECT_NE(getPage<LoadPage>(view, Constants::Sidebar::INDEX_LOAD), nullptr);
    EXPECT_NE(getPage<OverviewPage>(view, Constants::Sidebar::INDEX_OVERVIEW), nullptr);
    EXPECT_NE(getPage<EcusPage>(view, Constants::Sidebar::INDEX_ECUS), nullptr);
    EXPECT_NE(getPage<MessagesPage>(view, Constants::Sidebar::INDEX_MESSAGES), nullptr);
    EXPECT_NE(getPage<SignalsPage>(view, Constants::Sidebar::INDEX_SIGNALS), nullptr);
}

// ============================================================================
// 2. MODEL WIRING
// ============================================================================
TEST_F(DbcViewLogicTest, SetSourceModel_CreatesProxies)
{
    DbcView view;
    QStandardItemModel dummyModel;

    view.setSourceModel(&dummyModel);

    ASSERT_NE(view.getOverviewEcuProxy(), nullptr);
    ASSERT_NE(view.getOverviewMessageProxy(), nullptr);
    ASSERT_NE(view.getMessagesProxy(), nullptr);
    ASSERT_NE(view.getSignalsProxy(), nullptr);
    ASSERT_NE(view.getEcuTreeProxy(), nullptr);

    EXPECT_EQ(view.getOverviewEcuProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getOverviewMessageProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getMessagesProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getSignalsProxy()->sourceModel(), &dummyModel);
    EXPECT_EQ(view.getEcuTreeProxy()->sourceModel(), &dummyModel);
}

// ============================================================================
// 3. NAVIGATION LOGIC
// ============================================================================
TEST_F(DbcViewLogicTest, SidebarSwitch_ChangesStackIndex)
{
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);

    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_LOAD);

    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_MESSAGES);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_MESSAGES);

    invokeSlot(&view, "onSidebarSelectionChanged", Constants::Sidebar::INDEX_SIGNALS);
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_SIGNALS);
}

TEST_F(DbcViewLogicTest, SidebarSwitch_InvalidIndices_Ignored)
{
    DbcView view;
    auto* stack = view.findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);

    int initialIndex = stack->currentIndex();

    invokeSlot(&view, "onSidebarSelectionChanged", -1);
    EXPECT_EQ(stack->currentIndex(), initialIndex);

    invokeSlot(&view, "onSidebarSelectionChanged", 99);
    EXPECT_EQ(stack->currentIndex(), initialIndex);
}

// ============================================================================
// 4. FILTER LOGIC
// ============================================================================
TEST_F(DbcViewLogicTest, MessageFilter_UpdatesProxy)
{
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getMessagesProxy();
    ASSERT_NE(proxy, nullptr);

    invokeSlot(&view, "onMessageFilterTextChanged", "SpeedMsg");
    EXPECT_EQ(proxy->getSearchFilter(), "SpeedMsg");

    invokeSlot(&view, "onMessageSenderChanged", "EngineECU");
    EXPECT_EQ(proxy->getFilterMessageSender(), "EngineECU");
}

TEST_F(DbcViewLogicTest, SignalFilter_UpdatesProxy)
{
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getSignalsProxy();
    ASSERT_NE(proxy, nullptr);

    invokeSlot(&view, "onSignalFilterTextChanged", "RPM");
    EXPECT_EQ(proxy->getSearchFilter(), "RPM");

    invokeSlot(&view, "onSignalUnitChanged", "km/h");
    EXPECT_EQ(proxy->getSignalFilterUnit(), "km/h");
}

TEST_F(DbcViewLogicTest, EcuFilter_UpdatesProxy)
{
    DbcView view;
    QStandardItemModel dummyModel;
    view.setSourceModel(&dummyModel);

    auto* proxy = view.getEcuTreeProxy();
    ASSERT_NE(proxy, nullptr);

    invokeSlot(&view, "onEcuFilterTextChanged", "Gateway");
    EXPECT_EQ(proxy->getSearchFilter(), "Gateway");

    invokeSlot(&view, "onEcuFilterIndexChanged", 1);
    EXPECT_EQ(proxy->getFilterCategory(), 1);
}

// ============================================================================
// 5. ROBUSTNESS
// ============================================================================
TEST_F(DbcViewLogicTest, Slots_DoNotCrashWithoutModel)
{
    DbcView view;

    EXPECT_NO_THROW(invokeSlot(&view, "onMessageFilterTextChanged", "Text"));
    EXPECT_NO_THROW(invokeSlot(&view, "onSignalUnitChanged", "Unit"));
    EXPECT_NO_THROW(invokeSlot(&view, "onEcuFilterTextChanged", "Text"));
}

TEST_F(DbcViewLogicTest, SetSourceModel_NullHandled)
{
    DbcView view;
    EXPECT_NO_THROW(view.setSourceModel(nullptr));
}

TEST_F(DbcViewLogicTest, PublicAPI_SettersDoNotCrash)
{
    DbcView view;

    EXPECT_NO_THROW(view.setSignalUnits({"km/h", "rpm"}));
    EXPECT_NO_THROW(view.setAvailableSenders({"Engine", "Brake"}));
    EXPECT_NO_THROW(view.setNavigationEnabled(false));
}