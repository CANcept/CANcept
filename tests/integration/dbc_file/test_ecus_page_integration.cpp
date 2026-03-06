#include <gtest/gtest.h>
#include <QTreeView>
#include <QLineEdit>
#include <QComboBox>

// Module Includes
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/pages/ecus_page.hpp"
#include "dbc_file/constants.hpp"

// Core Widgets
#include "core/widgets/common/searchable_filter_widgets.hpp"

// Helpers
#include "core/widgets/common/styled_filter_bar.hpp"
#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class EcusPageIntegrationTest : public ::testing::Test {
protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<DbcComponent> component;
    EcusPage* ecusPage = nullptr;

    void SetUp() override {
        if (!QApplication::instance()) {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        QWidget* mainView = component->getView();
        mainView->resize(800, 600);
        mainView->show();

        ecusPage = mainView->findChild<EcusPage*>();
        ASSERT_NE(ecusPage, nullptr);
    }

    void TearDown() override {
        component->onStop();
        component.reset();
    }

    // --- Helpers for Widget-Access ---

    QTreeView* getTreeView() const
    {
        auto* container = ecusPage->findChild<Core::SearchableFilterTree*>();
        return container ? container->treeView() : nullptr;
    }

    Core::StyledFilterBar* getFilterBar() const
    {
        auto* container = ecusPage->findChild<Core::SearchableFilterTree*>();
        return container ? container->filterBar() : nullptr;
    }

    QLabel* getEmptyLabel() const
    {
        auto labels = ecusPage->findChildren<QLabel*>();
        for (auto* lbl : labels) {
            if (lbl->text() == Constants::EcusPage::EmptyLabelText) return lbl;
        }
        return nullptr;
    }
};

/**
 * Scenario: Load config with 2 ECUs.
 * Expectation: TreeView shows two top level items
 */
TEST_F(EcusPageIntegrationTest, PopulatesTreeWithEcus) {
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
        .node("EngineECU")
        .node("BrakeECU")
        .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    // 3. Assert
    QTreeView* tree = getTreeView();
    ASSERT_NE(tree, nullptr);
    QAbstractItemModel* model = tree->model();
    ASSERT_NE(model, nullptr);

    EXPECT_EQ(model->rowCount(), 2);


    QString name0 = model->index(0, 0).data().toString();
    QString name1 = model->index(1, 0).data().toString();

    bool hasEngine = (name0 == "EngineECU" || name1 == "EngineECU");
    bool hasBrake = (name0 == "BrakeECU" || name1 == "BrakeECU");

    EXPECT_TRUE(hasEngine);
    EXPECT_TRUE(hasBrake);
}

/**
 * @brief Test 2: Text-Filtering
 * Scenario: Search "Brake".
 * Expectation: "Engine" hidden.
 */
TEST_F(EcusPageIntegrationTest, FiltersEcusByText) {
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
        .node("EngineECU")
        .node("BrakeECU")
        .build();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTreeView* tree = getTreeView();
    Core::StyledFilterBar* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    // 2. Act: Simulate search
    filterBar->setSearchText("Brake");

    // 3. Assert
    EXPECT_EQ(tree->model()->rowCount(), 1);
    EXPECT_EQ(tree->model()->index(0, 0).data().toString(), "BrakeECU");
}

/**
 * @brief Test 3: Category-Filter (Filter for only sending ECUs)
 * Scenario: Active ECU (sends Msg) + passive ECU (no messages)
 * Set "Active only" filter
 */
TEST_F(EcusPageIntegrationTest, FiltersActiveEcus) {
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
        .node("ActiveECU")
        .node("PassiveECU")
        .message(TestHelpers::DbcMessageBuilder(0x100, "Msg").transmitter("ActiveECU"))
        .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTreeView* tree = getTreeView();
    Core::StyledFilterBar* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    // Pre-Check: both visible
    ASSERT_EQ(tree->model()->rowCount(), 2);

    // 2. Act: Set "Active only" filter
    filterBar->setCurrentFilterIndex(Constants::EcusPage::FilterActiveIndex);

    // 3. Assert
    EXPECT_EQ(tree->model()->rowCount(), 1);
    EXPECT_EQ(tree->model()->index(0, 0).data().toString(), "ActiveECU");
}

/**
 * @brief Test 4: Empty State Logic
 * Scenario: Load empty config.
 * Expectation: Label visible, tree invisible
 */
TEST_F(EcusPageIntegrationTest, ShowsEmptyStateOnNoData) {
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder().build();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "empty.dbc"));

    // 2. Assert
    QLabel* emptyLabel = getEmptyLabel();
    QTreeView* tree = getTreeView();

    ASSERT_NE(emptyLabel, nullptr);

    EXPECT_FALSE(emptyLabel->isHidden()) << "Empty label should be visible";
    EXPECT_TRUE(tree->isHidden()) << "TreeView should be hidden";
}

/**
 * @brief Test 5: Empty State at unsuccessful search
 * Scenario: ECU "Engine" is there, user searches "Banana"
 * Expectation: Tree hidden, empty label visible.
 */
TEST_F(EcusPageIntegrationTest, ShowsEmptyLabelOnNoSearchResults) {
    // 1. Arrange:
    auto config = TestHelpers::DbcConfigBuilder()
        .node("EngineECU")
        .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTreeView* tree = getTreeView();
    QLabel* emptyLabel = getEmptyLabel();
    Core::StyledFilterBar* filterBar = getFilterBar();

    // Pre-Check
    ASSERT_FALSE(tree->isHidden());
    ASSERT_TRUE(emptyLabel->isHidden());

    // 2. Act: Search for nonsense
    filterBar->setSearchText("Banana");

    // 3. Assert
    EXPECT_EQ(tree->model()->rowCount(), 0);

    EXPECT_FALSE(emptyLabel->isHidden())
        << "Empty Label should be visible when all items are filtered out";

    EXPECT_TRUE(tree->isHidden()) << "Tree should be hidden when all items are filtered out";

    // 4. Act (Reset): Delete search
    filterBar->setSearchText("");

    // Assert Reset
    EXPECT_TRUE(emptyLabel->isHidden());
    EXPECT_FALSE(tree->isHidden());
}

/**
 * @brief Test 6: Filter Reset at new file
 * Scenario: Set filter to "Active" + enter search text -> load new file
 * Expectation: Filter reset, new data is not being filtered
 */
TEST_F(EcusPageIntegrationTest, ResetsFilterOnNewLoad) {
    // 1. Arrange: Load config A and set filters
    mockBroker.triggerEvent(Core::DBCParsedEvent(
        TestHelpers::DbcConfigBuilder().node("EngineECU").node("Brakes").build(), "A.dbc"));

    QTreeView* tree = getTreeView();
    Core::StyledFilterBar* filterBar = getFilterBar();
    // Set filters
    filterBar->setSearchText("Anything");
    filterBar->setCurrentFilterIndex(Constants::EcusPage::FilterActiveIndex);

    // 2. Act: Load new config
    mockBroker.triggerEvent(Core::DBCParsedEvent(
        TestHelpers::DbcConfigBuilder().node("NodeB").build(), "B.dbc"));

    // 3. Assert
    EXPECT_EQ(filterBar->currentFilterData().toInt(), 0);
    EXPECT_EQ(tree->model()->rowCount(), 1);
    auto nodeIdx = tree->model()->index(0, Constants::Columns::EcuName);
    EXPECT_EQ(nodeIdx.data().toString(), "NodeB");
}

/**
 * @brief Test 7: Hierarchy-Check (ECU -> Message [Leaf])
 * Scenario: Load config with signals
 * Expectation:
 * - ECU is visible.
 * - Message is visible below ECU.
 * - Signals not visible as children (proxy hides for display in grid layout)
 */
TEST_F(EcusPageIntegrationTest, PopulatesTreeWithCorrectHierarchy) {
    // 1. Arrange: Full Hierarchy (ECU -> Message -> Signals)
    auto config = TestHelpers::DbcConfigBuilder()
        .node("EngineECU")
        .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg")
            .transmitter("EngineECU")
            .signal(TestHelpers::DbcSignalBuilder("Velocity"))
            .signal(TestHelpers::DbcSignalBuilder("RPM")))
        .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "hierarchy.dbc"));

    QTreeView* tree = getTreeView();
    QAbstractItemModel* model = tree->model();

    // 3. Assert: Level 1 (ECU)
    ASSERT_EQ(model->rowCount(), 1);
    QModelIndex ecuIdx = model->index(0, 0);
    EXPECT_EQ(ecuIdx.data().toString(), "EngineECU");

    // 4. Assert: Level 2 (Message)
    EXPECT_TRUE(model->hasChildren(ecuIdx));
    ASSERT_EQ(model->rowCount(ecuIdx), 1);

    QModelIndex msgIdx = model->index(0, 0, ecuIdx);
    EXPECT_EQ(msgIdx.data().toString(), "SpeedMsg");

    // 5. Assert: Level 3 (Signals below Message hidden)

    EXPECT_FALSE(model->hasChildren(msgIdx))
        << "Messages should be leafs in proxy";

    EXPECT_EQ(model->rowCount(msgIdx), 0)
        << "Signals should be hidden by proxy";
}