#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "monitoring/monitoring_component.hpp"
#include "monitoring/view/monitoring_view.hpp"
#include "monitoring/view/signal_list.hpp"
#include "monitoring/view/graph_list_view.hpp"

// Helpers
#include "../tests/helpers/dbc_examples.hpp"
#include "../tests/helpers/mock_event_broker.hpp"

class MonitoringComponentIntegrationTest : public ::testing::Test
{
protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<MonitoringComponent> component;
    MonitoringView* view = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<MonitoringComponent>(mockBroker);
        component->onStart();

        view = qobject_cast<MonitoringView*>(component->getView());
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

    Monitoring::GraphListView* getGraphListView() const
    {
        return view->findChild<Monitoring::GraphListView*>();
    }

    Monitoring::SignalList* getSignalList() const
    {
        return view->findChild<Monitoring::SignalList*>();
    }
};

/**
 * @brief Test 1: Initial navigation lock
 */
TEST_F(MonitoringComponentIntegrationTest, NavigationIsLockedInitially)
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

    EXPECT_FALSE(isTabEnabled(overviewIdx)) << "Overview Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(ecusIdx)) << "ECUs Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(msgIdx)) << "Messages Tab should initially be locked";
    EXPECT_FALSE(isTabEnabled(sigIdx)) << "Signals Tab should initially be locked";
    EXPECT_TRUE(isTabEnabled(loadIdx)) << "Load Page Tab always has to be active";
}

/**
 * @brief Test 2: Navigation unlocked at successful file load and interface conection
 */
TEST_F(MonitoringComponentIntegrationTest, NavigationUnlocksOnParseSuccess)
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