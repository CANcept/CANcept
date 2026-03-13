#include <gtest/gtest.h>
#include <QApplication>

// Module Includes
#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/dbc_view.hpp"
#include "dbc_file/view/pages/load_page.hpp"

// Core Includes
#include "core/widgets/sidebar.hpp"

// Helpers
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class DbcComponentIntegrationTest : public ::testing::Test
{
   protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<DbcComponent> component;
    DbcView* view = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        view = qobject_cast<DbcView*>(component->getView());
        ASSERT_NE(view, nullptr);

        view->resize(1024, 768);
        view->show();
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }

    // --- Helpers ---

    Core::Sidebar* getSidebar() const
    {
        return view->findChild<Core::Sidebar*>();
    }

    QStackedWidget* getStack() const
    {
        return view->findChild<QStackedWidget*>();
    }

    /**
     * @brief Finds Model-Index of a tab based on name of the tab
     */
    int findTabIndex(const QString& name) const
    {
        auto* sidebar = getSidebar();
        if (!sidebar || !sidebar->model()) return -1;

        QAbstractItemModel* model = sidebar->model();
        for (int i = 0; i < model->rowCount(); ++i)
        {
            QModelIndex idx = model->index(i, 0);
            if (idx.data(Qt::DisplayRole).toString().contains(name))
            {
                return i;
            }
        }
        return -1;
    }

    /**
     * @brief Checks if a tab is enabeld
     */
    bool isTabEnabled(int index)
    {
        auto* sidebar = getSidebar();
        if (!sidebar || !sidebar->model()) return false;

        QModelIndex idx = sidebar->model()->index(index, 0);
        return idx.flags() & Qt::ItemIsEnabled;
    }
};

/**
 * @brief Test 1: Initial navagation lock
 */
TEST_F(DbcComponentIntegrationTest, NavigationIsLockedInitially)
{
    int overviewIdx = findTabIndex(Constants::Sidebar::TitleOverview);
    int ecusIdx = findTabIndex(Constants::Sidebar::TitleEcus);
    int msgIdx = findTabIndex(Constants::Sidebar::TitleMessages);
    int sigIdx = findTabIndex(Constants::Sidebar::TitleSignals);
    int loadIdx = findTabIndex(Constants::Sidebar::TitleLoadNew);

    ASSERT_GE(overviewIdx, 0) << "Overview Tab not found in model";
    ASSERT_GE(ecusIdx, 0) << "ECUs Tab not found in model";
    ASSERT_GE(msgIdx, 0) << "Messages Tab not found in model";
    ASSERT_GE(sigIdx, 0) << "Signals Tab not found in model";
    ASSERT_GE(loadIdx, 0) << "Load Tab not found in model";

    EXPECT_FALSE(isTabEnabled(overviewIdx)) << "Overview Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(ecusIdx)) << "ECUs Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(msgIdx)) << "Messages Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(sigIdx)) << "Signals Tab should initially be locked";
    EXPECT_TRUE(isTabEnabled(loadIdx)) << "Load Page Tab always has to be active";
}

/**
 * @brief Test 2: Navigation unlocked at successful file load
 */
TEST_F(DbcComponentIntegrationTest, NavigationUnlocksOnParseSuccess)
{
    int overviewIdx = findTabIndex(Constants::Sidebar::TitleOverview);
    int ecusIdx = findTabIndex(Constants::Sidebar::TitleEcus);
    int msgIdx = findTabIndex(Constants::Sidebar::TitleMessages);
    int sigIdx = findTabIndex(Constants::Sidebar::TitleSignals);
    int loadIdx = findTabIndex(Constants::Sidebar::TitleLoadNew);
    ASSERT_GE(overviewIdx, 0) << "Overview Tab not found in model";
    ASSERT_GE(ecusIdx, 0) << "Overview Tab not found in model";
    ASSERT_GE(msgIdx, 0) << "Overview Tab not found in model";
    ASSERT_GE(sigIdx, 0) << "Overview Tab not found in model";
    ASSERT_GE(loadIdx, 0) << "Load Tab not found in model";
    ASSERT_FALSE(isTabEnabled(overviewIdx));  // Pre-Check

    // 2. Act: Successful file parsing
    auto config = TestHelpers::DbcExamples::simple();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    // 3. Assert: Jetzt frei
    EXPECT_TRUE(isTabEnabled(loadIdx))
        << "Navigation should be unlocked after successful file load.";
    EXPECT_TRUE(isTabEnabled(overviewIdx))
        << "Navigation should be unlocked after successful file load.";
    EXPECT_TRUE(isTabEnabled(ecusIdx))
        << "Navigation should be unlocked after successful file load.";
    EXPECT_TRUE(isTabEnabled(msgIdx))
        << "Navigation should be unlocked after successful file load.";
    EXPECT_TRUE(isTabEnabled(sigIdx))
        << "Navigation should be unlocked after successful file load.";
}

/**
 * @brief Test 4: Page switching
 */
TEST_F(DbcComponentIntegrationTest, SwitchingSidebarTabChangesPage)
{
    // 1. Arrange
    mockBroker.triggerEvent(Core::DBCParsedEvent(TestHelpers::DbcExamples::simple(), "test.dbc"));
    QApplication::processEvents();

    QStackedWidget* stack = getStack();
    Core::Sidebar* sidebar = getSidebar();

    ASSERT_EQ(stack->currentIndex(), 0);

    // 2. Act: simulate tab switch
    emit sidebar->tabSelected(Constants::Sidebar::INDEX_OVERVIEW);
    QApplication::processEvents();

    // 3. Assert
    EXPECT_EQ(stack->currentIndex(), Constants::Sidebar::INDEX_OVERVIEW)
        << "StackedWidget should have switched to overviewpage";
}

/**
 * @brief Test 5: LoadPage Reset when leaving load page
 */
TEST_F(DbcComponentIntegrationTest, ResetsLoadPageStatusWhenLeaving)
{
    // 1. Arrange: Create error
    mockBroker.triggerEvent(Core::DBCParseErrorEvent("Test error", "test.dbc"));
    QApplication::processEvents();

    LoadPage* loadPage = view->findChild<LoadPage*>();
    QLabel* statusLbl = loadPage->findChild<QLabel*>("StatusLabel");

    ASSERT_TRUE(statusLbl->isVisible());

    Core::Sidebar* sidebar = getSidebar();

    // 2. Act: Switch to overgiew
    emit sidebar->tabSelected(Constants::Sidebar::INDEX_OVERVIEW);
    QApplication::processEvents();

    // 3. Act: Switch back to load page
    emit sidebar->tabSelected(Constants::Sidebar::INDEX_LOAD);
    QApplication::processEvents();

    // 4. Assert: LoadPage status label has to be cleared
    EXPECT_FALSE(statusLbl->isVisible());
    EXPECT_TRUE(statusLbl->text().isEmpty());
}

TEST_F(DbcComponentIntegrationTest, IgnoresBrokerEventsAfterStop)
{
    auto* model = component->getModel();
    ASSERT_NE(model, nullptr);

    mockBroker.triggerEvent(Core::DBCParsedEvent(TestHelpers::DbcExamples::simple(), "first.dbc"));
    QApplication::processEvents();

    const int rowsBeforeStop = model->rowCount(QModelIndex());
    ASSERT_GT(rowsBeforeStop, 1);

    component->onStop();

    // After stop, publish() should not deliver events to released subscriptions.
    EXPECT_NO_THROW(
        mockBroker.publish(Core::DBCParsedEvent(TestHelpers::DbcExamples::empty(), "second.dbc")));
    EXPECT_NO_THROW(mockBroker.publish(Core::DBCParseErrorEvent("ignored", "second.dbc")));
    QApplication::processEvents();

    EXPECT_EQ(model->rowCount(QModelIndex()), rowsBeforeStop);
}
