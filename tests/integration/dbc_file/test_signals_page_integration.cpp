#include <gtest/gtest.h>

#include <QComboBox>
#include <QLineEdit>
#include <QTableView>

// Module Includes
#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/pages/signals_page.hpp"

// Core Widgets
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"

// Helpers
#include <ranges>

#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class SignalsPageIntegrationTest : public ::testing::Test
{
   protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<DbcComponent> component;
    SignalsPage* signalsPage = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        QWidget* mainView = component->getView();
        mainView->resize(1024, 768);
        mainView->show();

        signalsPage = mainView->findChild<SignalsPage*>();
        ASSERT_NE(signalsPage, nullptr);
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }

    // --- Helpers ---

    QTableView* getTable() const
    {
        auto* container = signalsPage->findChild<Core::SearchableFilterTable*>();
        return container ? container->tableView() : nullptr;
    }

    Core::StyledFilterBar* getFilterBar() const
    {
        auto* container = signalsPage->findChild<Core::SearchableFilterTable*>();
        return container ? container->filterBar() : nullptr;
    }

    QLabel* getEmptyLabel() const
    {
        auto labels = signalsPage->findChildren<QLabel*>();
        for (auto* lbl : labels)
        {
            if (lbl->text() == Constants::SignalsPage::EmptyLabelText) return lbl;
        }
        return nullptr;
    }
};

/**
 * @brief Test 1: Filling
 * Scenario: Load config with 2 signals
 * Expectation: Table shows 2 rows
 */
TEST_F(SignalsPageIntegrationTest, PopulatesTableWithSignals)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "Msg")
                                   .signal(TestHelpers::DbcSignalBuilder("SigA"))
                                   .signal(TestHelpers::DbcSignalBuilder("SigB")))
                      .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    // 3. Assert
    QTableView* table = getTable();
    ASSERT_NE(table, nullptr);
    ASSERT_NE(table->model(), nullptr);

    EXPECT_EQ(table->model()->rowCount(), 2);

    // Check names
    QString name0 = table->model()->index(0, Constants::Columns::SigName).data().toString();
    QString name1 = table->model()->index(1, Constants::Columns::SigName).data().toString();

    EXPECT_TRUE(name0 == "SigA" || name1 == "SigA");
    EXPECT_TRUE(name0 == "SigB" || name1 == "SigB");
}

/**
 * @brief Test 2: Text Search
 * Scenario: Search for "Speed"
 * Expectation: "RPM" hidden, "Speed" shown.
 */
TEST_F(SignalsPageIntegrationTest, FiltersSignalsByText)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "Msg")
                                   .signal(TestHelpers::DbcSignalBuilder("Speed"))
                                   .signal(TestHelpers::DbcSignalBuilder("RPM")))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getTable();
    Core::StyledFilterBar* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    // 2. Act
    filterBar->setSearchText("Speed");

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 1);
    EXPECT_EQ(table->model()->index(0, Constants::Columns::SigName).data().toString(), "Speed");
}

/**
 * @brief Test 3: Unit Filter
 * Scenario: Signals with Units "km/h" and "rpm".
 * Set filter to "km/h"
 */
TEST_F(SignalsPageIntegrationTest, FiltersSignalsByUnit)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "Msg")
                                   .signal(TestHelpers::DbcSignalBuilder("Speed").unit("km/h"))
                                   .signal(TestHelpers::DbcSignalBuilder("Engine").unit("rpm")))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getTable();
    Core::StyledFilterBar* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    // Pre-Check
    ASSERT_EQ(table->model()->rowCount(), 2);

    // 2. Act: Select "km/h"
    EXPECT_NO_THROW(filterBar->setCurrentFilterText("km/h"));

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 1);

    // Check if right signal is shown
    auto idxName = table->model()->index(0, Constants::Columns::SigName);
    EXPECT_EQ(idxName.data().toString(), "Speed");
}

/**
 * @brief Test 4: Empty State
 * Scenario: Config without signals
 * Expectation: Table hidden, empty label shown
 */
TEST_F(SignalsPageIntegrationTest, ShowsEmptyStateOnNoSignals)
{
    // 1. Arrange: Message without signals
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "EmptyMsg"))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "empty.dbc"));

    // 2. Assert
    QLabel* emptyLabel = getEmptyLabel();
    QTableView* table = getTable();

    ASSERT_NE(emptyLabel, nullptr);
    ASSERT_NE(table, nullptr);

    EXPECT_FALSE(emptyLabel->isHidden()) << "Empty Label should be visible";
    EXPECT_TRUE(table->isHidden()) << "Table should be hidden";
}
/**
 * @brief Test 6: Empty State at unsuccessful search
 * Scenario: Data is there, but user searches for "BananaXYZ".
 * Expectation: Everything is filtered out -> table hidden -> label visible
 */
TEST_F(SignalsPageIntegrationTest, ShowsEmptyLabelOnNoSearchResults)
{
    // 1. Arrange: Load Config with data
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "Msg")
                                   .signal(TestHelpers::DbcSignalBuilder("Speed")))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getTable();
    QLabel* emptyLabel = getEmptyLabel();
    Core::StyledFilterBar* filterBar = getFilterBar();

    // Pre-Check:
    ASSERT_FALSE(table->isHidden());
    ASSERT_TRUE(emptyLabel->isHidden());

    // 2. Act: Search for text
    filterBar->setSearchText("BananaXYZ");

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 0) << "Proxy should have filtered everything out";

    EXPECT_FALSE(emptyLabel->isHidden())
        << "Empty Label should be visible when proxy hides all items.";

    EXPECT_TRUE(table->isHidden()) << "Table should be hidden when empty";

    // 4. Act: Delete Search -> Table appears again
    filterBar->setSearchText("");

    EXPECT_TRUE(emptyLabel->isHidden()) << "Label should vanish when search is deleted";
    EXPECT_FALSE(table->isHidden()) << "Table should be visible again when search is deleted";
}

/**
 * @brief Test 5: Filter Reset at new File loaded
 * Scenario:
 * 1. Load File A -> Set filter to "km/h" + set search text
 * 2. Load File B
 * 3. Check for filter reset
 */
TEST_F(SignalsPageIntegrationTest, ResetsUnitFilterOnNewLoad)
{
    // 1. Arrange: File A with km/h
    auto configA = TestHelpers::DbcConfigBuilder()
                       .message(TestHelpers::DbcMessageBuilder(0x100, "A")
                                    .signal(TestHelpers::DbcSignalBuilder("S1").unit("km/h")))
                       .build();
    mockBroker.triggerEvent(Core::DBCParsedEvent(configA, "A.dbc"));

    Core::StyledFilterBar* filterBar = getFilterBar();
    filterBar->setCurrentFilterText("km/h");

    // 2. Act: Load File B
    auto configB = TestHelpers::DbcConfigBuilder()
                       .message(TestHelpers::DbcMessageBuilder(0x200, "B")
                                    .signal(TestHelpers::DbcSignalBuilder("S2").unit("Volt")))
                       .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(configB, "B.dbc"));

    // 3. Assert
    EXPECT_EQ(filterBar->currentFilterText(), Constants::SignalsPage::FilterAllText)
        << "Unit filter should return to default after new file is loaded";
}