#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QListView>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTest>

#include "app_root/delegate/app_root_delegate.hpp"
#include "app_root/entry_point/app_root.hpp"
#include "app_root/model/app_root_model.hpp"
#include "app_root/model/settings_model.hpp"
#include "app_root/service/settings_service.hpp"
#include "app_root/view/app_root_view.hpp"
#include "core/event/lifecycle_event.hpp"
#include "core/interface/i_tab_component.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/mock_tab_component.hpp"

using namespace AppRoot;
using namespace TestHelpers;
using ::testing::_;
using ::testing::AnyNumber;

class AppRootIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        broker = std::make_shared<MockEventBroker>();
        EXPECT_CALL(*broker, _subscribeEvent(_)).Times(AnyNumber());
        EXPECT_CALL(*broker, _publishEvent(_, _)).Times(AnyNumber());

        service = std::make_unique<SettingsService>(*broker);
        settingsModel = std::make_unique<SettingsModel>(*service, *broker);

        model = std::make_unique<AppRootModel>();
        delegate = std::make_unique<AppRootDelegate>();
        view = std::make_unique<AppRootView>();
        view->setDelegate(delegate.get());
        view->setModel(model.get());
    }

    void TearDown() override
    {
        view.reset();
        tabs.clear();
        settingsModel.reset();
        service.reset();
        delegate.reset();
        model.reset();
        broker.reset();
    }

    template <typename T>
    auto addTab(const QString& id, const QString& title) -> T*
    {
        auto tab = std::make_unique<T>(*broker, id, title);
        auto* ptr = tab.get();
        model->addTab(ptr);
        tabs.push_back(std::move(tab));
        return ptr;
    }

    void start() const
    {
        view->setSettingsModel(settingsModel.get());
        view->show();
        QTest::qWait(10);
    }

    void bootstrap(const int numTabs = 2)
    {
        for (int i = 0; i < numTabs; ++i)
            addTab<RealWidgetTabComponent>("t" + QString::number(i + 1),
                                           "Tab " + QString::number(i + 1));
        start();
    }

    auto contentStack() const -> QStackedWidget*
    {
        return view->findChild<QStackedWidget*>();
    }
    auto tabListView() const -> QListView*
    {
        return view->findChild<QListView*>();
    }
    auto settingsButton() const -> QPushButton*
    {
        return view->findChild<QPushButton*>(QString("settingsButton"), Qt::FindDirectChildrenOnly);
    }
    void selectTab(const int index) const
    {
        tabListView()->setCurrentIndex(model->index(index, 0));
        QTest::qWait(10);
    }

    std::shared_ptr<MockEventBroker> broker;
    std::unique_ptr<AppRootModel> model;
    std::unique_ptr<AppRootDelegate> delegate;
    std::unique_ptr<AppRootView> view;
    std::unique_ptr<SettingsService> service;
    std::unique_ptr<SettingsModel> settingsModel;
    std::vector<std::unique_ptr<Core::ITabComponent>> tabs;
};

// Publishing AppStartedEvent through the broker calls onStart() on every tab.
TEST_F(AppRootIntegrationTest, Flow_Startup_AppStartedEvent_TriggersOnStartOnAllTabs)
{
    const auto* t1 = addTab<LifecycleTrackingTabComponent>("t1", "Tab 1");
    const auto* t2 = addTab<LifecycleTrackingTabComponent>("t2", "Tab 2");

    start();
    broker->publish<Core::AppStartedEvent>({});

    EXPECT_EQ(t1->startCount, 1);
    EXPECT_EQ(t2->startCount, 1);
}

// Publishing AppStoppedEvent calls onStop() on every tab that was started.
TEST_F(AppRootIntegrationTest, Flow_Shutdown_AppStoppedEvent_TriggersOnStopOnAllTabs)
{
    const auto* t1 = addTab<LifecycleTrackingTabComponent>("t1", "Tab 1");
    const auto* t2 = addTab<LifecycleTrackingTabComponent>("t2", "Tab 2");
    start();

    broker->publish<Core::AppStartedEvent>({});
    broker->publish<Core::AppStoppedEvent>({});

    EXPECT_EQ(t1->stopCount, 1);
    EXPECT_EQ(t2->stopCount, 1);
}

// Emitting updated() on a tab propagates through the model and reaches the view
TEST_F(AppRootIntegrationTest, Flow_TabUpdated_PropagatesViaModelToView)
{
    auto* tab = addTab<LifecycleTrackingTabComponent>("t1", "Tab 1");
    start();

    const QSignalSpy spy(model.get(), &AppRootModel::dataChanged);
    emit tab->updated();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).value<QModelIndex>().row(), 0);
}

// After start(), the first tab is auto-selected and the stack matches.
TEST_F(AppRootIntegrationTest, Flow_InitialStateAndNavigation)
{
    bootstrap(3);

    // 3 tabs + settings view
    ASSERT_EQ(contentStack()->count(), 4);
    EXPECT_EQ(tabListView()->selectionModel()->currentIndex().row(), 0);
    EXPECT_EQ(contentStack()->currentIndex(), 0);

    for (int i = 0; i < 3; ++i)
    {
        selectTab(i);
        EXPECT_EQ(contentStack()->currentIndex(), i) << "Stack mismatch at tab " << i;
    }

    // All data roles are consistent for the same tab
    const QModelIndex idx = model->index(1, 0);
    EXPECT_EQ(model->data(idx, Qt::DisplayRole).toString(), "Tab 2");
    EXPECT_EQ(model->data(idx, AppRootModel::IdRole).toString(), "t2");
    EXPECT_EQ(model->data(idx, AppRootModel::ComponentRole).value<Core::ITabComponent*>(),
              tabs[1].get());
}

// Removing and replacing tabs updates both the stack count and model data.
TEST_F(AppRootIntegrationTest, Flow_MutationsAndDeselectionPrevention)
{
    bootstrap(3);

    model->removeTab("t2");
    QTest::qWait(10);
    ASSERT_EQ(contentStack()->count(), 3);

    auto replacement = std::make_unique<MockTabComponent>(*broker, "replaced", "Replaced");
    model->replaceTab(tabs[0].get(), replacement.get());
    QTest::qWait(10);
    EXPECT_EQ(model->data(model->index(0), AppRootModel::IdRole).toString(), "replaced");
    EXPECT_EQ(model->componentAt(0), replacement.get());
    tabs.push_back(std::move(replacement));

    selectTab(0);
    tabListView()->selectionModel()->clearSelection();
    QTest::qWait(20);
    EXPECT_TRUE(tabListView()->selectionModel()->currentIndex().isValid());
}

// Open/close the settings panel 3 times from a non-first tab: each open shows
TEST_F(AppRootIntegrationTest, Flow_SettingsToggleCycleRestoresActiveTab)
{
    bootstrap(3);

    const int settingsIdx = contentStack()->count() - 1;
    selectTab(2);
    ASSERT_EQ(contentStack()->currentIndex(), 2);

    for (int cycle = 0; cycle < 3; ++cycle)
    {
        settingsButton()->click();
        QTest::qWait(10);
        EXPECT_EQ(contentStack()->currentIndex(), settingsIdx)
            << "Settings not shown on open, cycle " << cycle;

        settingsButton()->click();
        QTest::qWait(10);
        EXPECT_EQ(contentStack()->currentIndex(), 2)
            << "Tab 2 not restored on close, cycle " << cycle;
    }
}

// With settings open, clicking a tab closes settings and shows that tab.
TEST_F(AppRootIntegrationTest, Flow_SettingsClosedByTabClick_NavigationContinues)
{
    bootstrap(3);

    const int settingsIdx = contentStack()->count() - 1;

    settingsButton()->click();
    QTest::qWait(10);
    ASSERT_EQ(contentStack()->currentIndex(), settingsIdx);

    selectTab(2);
    EXPECT_EQ(contentStack()->currentIndex(), 2);

    selectTab(0);
    EXPECT_EQ(contentStack()->currentIndex(), 0);
    selectTab(1);
    EXPECT_EQ(contentStack()->currentIndex(), 1);
}

TEST(AppRootEntryPointTest, AppRoot_ConstructsAndDestructsCleanly)
{
    EXPECT_NO_THROW({ AppRoot::AppRoot appRoot; });
}
