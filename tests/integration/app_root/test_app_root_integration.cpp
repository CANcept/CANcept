#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QLabel>
#include <QListView>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTest>

#include "app_root/constants.hpp"
#include "app_root/delegate/app_root_delegate.hpp"
#include "app_root/model/app_root_model.hpp"
#include "app_root/model/settings_model.hpp"
#include "app_root/service/settings_service.hpp"
#include "app_root/view/app_root_view.hpp"
#include "core/dto/setting_dto.hpp"
#include "core/event/theme_event.hpp"
#include "core/interface/i_tab_component.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/mock_tab_component.hpp"

using namespace AppRoot;
using namespace TestHelpers;
using ::testing::_;
using ::testing::AnyNumber;

// =============================================================================
// TestWidgetTab — a tab component backed by a real QLabel for stack tests
// =============================================================================

class TestWidgetTab : public Core::ITabComponent
{
   public:
    TestWidgetTab(Core::IEventBroker& broker, const QString& id, const QString& title)
        : ITabComponent(broker, id, title), m_widget(new QLabel(id))
    {
    }

    ~TestWidgetTab() override
    {
        // QPointer becomes null when Qt deletes the widget (e.g. via QStackedWidget destructor)
        if (m_widget)
        {
            delete m_widget;
        }
    }

    void onStart() override {}
    void onStop() override {}
    auto getView() -> QWidget* override
    {
        return m_widget;
    }

   private:
    QPointer<QLabel> m_widget;
};

// =============================================================================
// Fixture
// =============================================================================

class AppRootIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        broker = std::make_shared<MockEventBroker>();
        EXPECT_CALL(*broker, _subscribeEvent(_)).Times(AnyNumber());
        EXPECT_CALL(*broker, _publishEvent(_, _)).Times(AnyNumber());

        model = std::make_unique<AppRootModel>();
        delegate = std::make_unique<AppRootDelegate>();
        view = std::make_unique<AppRootView>();
        view->setDelegate(delegate.get());
        // NOTE: setModel() is NOT called here — call buildView() after adding tabs
        // to match production order (AppRoot::bootstrap adds tabs before setModel).
    }

    void TearDown() override
    {
        // Destroy view first so QStackedWidget deletes reparented tab widgets.
        // TestWidgetTab destructors then see a null QPointer and skip double-free.
        view.reset();
        tabs.clear();
        settingsModel.reset();
        service.reset();
        delegate.reset();
        model.reset();
        broker.reset();
    }

    // Call AFTER all tabs and optional setupSettings() to wire model into view.
    // Mirrors production order: tabs added to model first, then setModel().
    void buildView()
    {
        view->setModel(model.get());
        QTest::qWait(10);
    }

    void setupSettings()
    {
        service = std::make_unique<SettingsService>(*broker);
        settingsModel = std::make_unique<SettingsModel>(*service, *broker);
        view->setSettingsModel(settingsModel.get());
    }

    // Add a TestWidgetTab (real widget) and keep ownership in this fixture.
    auto addRealTab(const QString& id, const QString& title) -> TestWidgetTab*
    {
        auto tab = std::make_unique<TestWidgetTab>(*broker, id, title);
        auto* ptr = tab.get();
        model->addTab(ptr);
        tabs.push_back(std::move(tab));
        return ptr;
    }

    // Navigate to a tab by index without triggering the deselection-prevention handler.
    // Uses QAbstractItemView::setCurrentIndex which does not emit selectionChanged.
    void selectTab(int index)
    {
        tabListView()->setCurrentIndex(model->index(index, 0));
        QTest::qWait(10);
    }

    // Accessors via findChild to avoid coupling to private members
    auto contentStack() const -> QStackedWidget*
    {
        return view->findChild<QStackedWidget*>();
    }

    auto tabListView() const -> QListView*
    {
        return view->findChild<QListView*>();
    }

    // The settings button is the only QPushButton that is a direct child of AppRootView.
    auto settingsButton() const -> QPushButton*
    {
        return view->findChild<QPushButton*>(QString(), Qt::FindDirectChildrenOnly);
    }

    std::shared_ptr<MockEventBroker> broker;
    std::unique_ptr<AppRootModel> model;
    std::unique_ptr<AppRootDelegate> delegate;
    std::unique_ptr<AppRootView> view;
    std::unique_ptr<SettingsService> service;
    std::unique_ptr<SettingsModel> settingsModel;
    std::vector<std::unique_ptr<Core::ITabComponent>> tabs;
};

// =============================================================================
// Tests: AppRootModel ↔ AppRootView integration
// =============================================================================

// Adding three tabs with real views fills the content stack to count == 3.
TEST_F(AppRootIntegrationTest, Flow_AddTabs_ContentStackCountMatchesTabCount)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    ASSERT_NE(contentStack(), nullptr);
    // Stack has 3 tabs + 1 settings view
    EXPECT_EQ(contentStack()->count(), 4);
}

// After setModel with tabs already added, the first tab is auto-selected.
TEST_F(AppRootIntegrationTest, Flow_SetModel_FirstTabAutoSelectedInListView)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    setupSettings();
    buildView();

    EXPECT_TRUE(tabListView()->selectionModel()->currentIndex().isValid());
    EXPECT_EQ(tabListView()->selectionModel()->currentIndex().row(), 0);
}

// Changing the selection model row switches the content stack to match.
TEST_F(AppRootIntegrationTest, Flow_TabSelectionChange_ContentStackFollowsSelection)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    selectTab(2);

    EXPECT_EQ(contentStack()->currentIndex(), 2);
}

// Navigating tab 0 → tab 1 → tab 2 in sequence updates the stack at each step.
TEST_F(AppRootIntegrationTest, Flow_SequentialTabNavigation_StackAlwaysMatchesSelection)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    for (int i = 0; i < 3; ++i)
    {
        selectTab(i);
        EXPECT_EQ(contentStack()->currentIndex(), i) << "Stack mismatch at tab index " << i;
    }
}

// Removing a tab via the model shrinks the content stack by one.
TEST_F(AppRootIntegrationTest, Flow_RemoveTab_ContentStackCountDecreases)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    // 3 tabs + settings view
    ASSERT_EQ(contentStack()->count(), 4);

    model->removeTab("t2");
    QTest::qWait(10);

    EXPECT_EQ(contentStack()->count(), 3);
}

// replaceTab updates the model data at that row to the new component.
TEST_F(AppRootIntegrationTest, Flow_ReplaceTab_ModelReturnsNewComponentAndId)
{
    auto tab1 = std::make_unique<MockTabComponent>(*broker, "old_id", "Old Tab");
    auto tab2 = std::make_unique<MockTabComponent>(*broker, "new_id", "New Tab");
    auto* rawOld = tab1.get();
    auto* rawNew = tab2.get();

    model->addTab(rawOld);
    setupSettings();
    buildView();

    ASSERT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->data(model->index(0), AppRootModel::IdRole).toString(), "old_id");

    model->replaceTab(rawOld, rawNew);
    QTest::qWait(10);

    EXPECT_EQ(model->data(model->index(0), AppRootModel::IdRole).toString(), "new_id");
    EXPECT_EQ(model->componentAt(0), rawNew);
}

// All three data roles (Display, Id, Component) return consistent values for the same tab.
TEST_F(AppRootIntegrationTest, Flow_ModelData_AllRolesConsistentForSameTab)
{
    auto tab = std::make_unique<MockTabComponent>(*broker, "my_tab", "My Tab");
    auto* rawTab = tab.get();
    model->addTab(rawTab);
    setupSettings();
    buildView();

    const QModelIndex idx = model->index(0, 0);

    EXPECT_EQ(model->data(idx, Qt::DisplayRole).toString(), "My Tab");
    EXPECT_EQ(model->data(idx, AppRootModel::IdRole).toString(), "my_tab");
    auto* component = model->data(idx, AppRootModel::ComponentRole).value<Core::ITabComponent*>();
    EXPECT_EQ(component, rawTab);
}

// =============================================================================
// Tests: Settings button state machine
// =============================================================================

// Clicking the settings button shows the settings view (last widget in the stack).
TEST_F(AppRootIntegrationTest, Flow_SettingsButton_Click_ShowsSettingsView)
{
    addRealTab("t1", "Tab 1");
    setupSettings();
    buildView();

    const int settingsIdx = contentStack()->count() - 1;
    ASSERT_NE(settingsButton(), nullptr);
    settingsButton()->click();
    QTest::qWait(20);

    EXPECT_EQ(contentStack()->currentIndex(), settingsIdx);
}

// Clicking settings twice returns to the tab that was active before settings opened.
TEST_F(AppRootIntegrationTest, Flow_SettingsButton_ToggleTwice_RestoresLastActiveTab)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    setupSettings();
    buildView();

    // Navigate to tab 1
    selectTab(1);
    ASSERT_EQ(contentStack()->currentIndex(), 1);

    settingsButton()->click();  // open
    QTest::qWait(10);
    settingsButton()->click();  // close
    QTest::qWait(10);

    EXPECT_EQ(contentStack()->currentIndex(), 1);
}

// With settings open, clicking a tab closes settings and shows that tab.
TEST_F(AppRootIntegrationTest, Flow_SettingsOpen_ThenTabClick_ClosesSettingsAndShowsClickedTab)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    setupSettings();
    buildView();

    settingsButton()->click();  // open settings
    QTest::qWait(10);
    ASSERT_EQ(contentStack()->currentIndex(), contentStack()->count() - 1);

    // Simulate clicking tab index 1
    selectTab(1);

    EXPECT_EQ(contentStack()->currentIndex(), 1);
    EXPECT_NE(contentStack()->currentIndex(), contentStack()->count() - 1);
}

// Opening settings three times in sequence always shows the settings view.
TEST_F(AppRootIntegrationTest, Flow_SettingsButton_RepeatedOpenClose_AlwaysWorksCorrectly)
{
    addRealTab("t1", "Tab 1");
    setupSettings();
    buildView();

    const int settingsIdx = contentStack()->count() - 1;

    for (int cycle = 0; cycle < 3; ++cycle)
    {
        settingsButton()->click();  // open
        QTest::qWait(10);
        EXPECT_EQ(contentStack()->currentIndex(), settingsIdx)
            << "Settings not shown on cycle " << cycle;

        settingsButton()->click();  // close
        QTest::qWait(10);
        EXPECT_NE(contentStack()->currentIndex(), settingsIdx)
            << "Settings still shown after close on cycle " << cycle;
    }
}

// Navigate to tab 2, open settings, close — the previously active tab 2 is restored.
TEST_F(AppRootIntegrationTest, Flow_SettingsCloseRestoresTabEvenAfterNavigatingFirst)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    // Navigate to tab 2 (index 2)
    selectTab(2);
    ASSERT_EQ(contentStack()->currentIndex(), 2);

    settingsButton()->click();  // open
    QTest::qWait(10);
    settingsButton()->click();  // close
    QTest::qWait(10);

    EXPECT_EQ(contentStack()->currentIndex(), 2);
}

// =============================================================================
// Tests: deselection prevention
// =============================================================================

// Attempting to clear tab selection while not in settings mode restores the previous selection.
TEST_F(AppRootIntegrationTest, Flow_ClearSelection_Prevented_SelectionAlwaysValid)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    setupSettings();
    buildView();

    selectTab(1);

    tabListView()->selectionModel()->clearSelection();
    QTest::qWait(20);

    // The selection should be restored — cannot end up with no tab selected
    EXPECT_TRUE(tabListView()->selectionModel()->currentIndex().isValid());
}

// =============================================================================
// Tests: complex combined flows
// =============================================================================

// Full flow: add tabs, navigate, open settings, close, navigate again — everything stays
// consistent.
TEST_F(AppRootIntegrationTest, Flow_FullNavigation_AddTabsNavigateSettingsNavigate_Consistent)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    addRealTab("t3", "Tab 3");
    setupSettings();
    buildView();

    // Navigate t0 → t2
    selectTab(2);
    ASSERT_EQ(contentStack()->currentIndex(), 2);

    // Open and close settings
    settingsButton()->click();
    QTest::qWait(10);
    settingsButton()->click();
    QTest::qWait(10);
    ASSERT_EQ(contentStack()->currentIndex(), 2);

    // Navigate to t1
    selectTab(1);
    EXPECT_EQ(contentStack()->currentIndex(), 1);
}

// Removing the currently displayed tab switches to another tab (not out of bounds).
TEST_F(AppRootIntegrationTest, Flow_RemoveCurrentTab_StackRemainsValid)
{
    addRealTab("t1", "Tab 1");
    addRealTab("t2", "Tab 2");
    setupSettings();
    buildView();

    // Select tab 0
    selectTab(0);

    model->removeTab("t1");
    QTest::qWait(20);

    // Stack must still have a valid current index after removal
    // (1 remaining tab + 1 settings view = 2)
    EXPECT_EQ(contentStack()->count(), 2);
    EXPECT_GE(contentStack()->currentIndex(), 0);
}
