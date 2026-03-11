#include <gtest/gtest.h>

#include <QTableView>

// Module Includes
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/dbc_component.hpp"
#include "dbc_file/view/pages/messages_page.hpp"

// Helpers
#include <QMouseEvent>

#include "tests/helpers/dbc_config_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace DbcFile;
using namespace testing;

class MessagesPageIntegrationTest : public ::testing::Test
{
   protected:
    int argc = 0;
    char** argv = nullptr;
    std::unique_ptr<QApplication> app;
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<DbcComponent> component;
    MessagesPage* messagesPage = nullptr;

    void SetUp() override
    {
        if (!QApplication::instance())
        {
            app = std::make_unique<QApplication>(argc, argv);
        }
        EXPECT_CALL(mockBroker, _subscribeEvent(_)).WillRepeatedly(Return());

        component = std::make_unique<DbcComponent>(mockBroker);
        component->onStart();

        // Prepare view
        QWidget* mainView = component->getView();
        mainView->resize(1024, 768);
        mainView->show();

        messagesPage = mainView->findChild<MessagesPage*>();
        ASSERT_NE(messagesPage, nullptr);
    }

    void TearDown() override
    {
        component->onStop();
        component.reset();
    }

    // --- Helper Methods ---

    QTableView* getMasterTable() const
    {
        auto* container = messagesPage->findChild<Core::SearchableFilterTable*>();
        return container ? container->tableView() : nullptr;
    }

    Core::StyledFilterBar* getFilterBar() const
    {
        auto* container = messagesPage->findChild<Core::SearchableFilterTable*>();
        return container ? container->filterBar() : nullptr;
    }

    MessageDetailView* getDetailView() const
    {
        return messagesPage->findChild<MessageDetailView*>();
    }
};

/**
 * @brief Test 1: Population
 * Scenario: Parse 3 Messages
 * Expectation: Table has 3 rows with correct names
 */
TEST_F(MessagesPageIntegrationTest, PopulatesMasterTable)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg"))
                      .message(TestHelpers::DbcMessageBuilder(0x101, "TorqueMsg"))
                      .message(TestHelpers::DbcMessageBuilder(0x102, "StatusMsg"))
                      .build();

    // 2. Act
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    // 3. Assert
    QTableView* table = getMasterTable();
    ASSERT_NE(table, nullptr);
    ASSERT_NE(table->model(), nullptr);

    EXPECT_EQ(table->model()->rowCount(), 3);

    // Check if names are correct
    auto idx0 = table->model()->index(0, Constants::Columns::MsgName);
    EXPECT_EQ(idx0.data().toString(), "SpeedMsg");
}

/**
 * @brief Test 2: Filtering
 * Scenario: Filter for Text "Torque".
 * Expectation: Only one row visible.
 */
TEST_F(MessagesPageIntegrationTest, FiltersMasterTable)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg"))
                      .message(TestHelpers::DbcMessageBuilder(0x101, "TorqueMsg"))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getMasterTable();
    Core::StyledFilterBar* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    // Pre-Check
    EXPECT_EQ(table->model()->rowCount(), 2);

    // 2. Act: Set filter
    filterBar->setSearchText("Torque");

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 1);
    auto idx0 = table->model()->index(0, Constants::Columns::MsgName);
    EXPECT_EQ(idx0.data().toString().toStdString(), "TorqueMsg");
}

/**
 * @brief Test 3: Sender-Kategorie Filterung
 * Szenario: Filtern nach "EngineECU".
 * Erwartung: Nur Messages von diesem Sender sichtbar.
 */
TEST_F(MessagesPageIntegrationTest, FiltersBySenderCategory)
{
    // 1. Arrange:
    auto config =
        TestHelpers::DbcConfigBuilder()
            .node("EngineECU")
            .node("BrakeECU")
            .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg").transmitter("EngineECU"))
            .message(TestHelpers::DbcMessageBuilder(0x200, "BrakeMsg").transmitter("BrakeECU"))
            .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    QTableView* table = getMasterTable();
    Core::StyledFilterBar* filterBar = getFilterBar();

    ASSERT_NE(table, nullptr);
    ASSERT_NE(filterBar, nullptr) << "FilterBar not found!";

    // Pre-Check
    ASSERT_EQ(table->model()->rowCount(), 2);

    // 2. Act: Select engine "EngineECU" from filtering combobox
    filterBar->setCurrentFilterText("EngineECU");

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 1);
    auto idx = table->model()->index(0, Constants::Columns::MsgSender);
    EXPECT_EQ(idx.data().toString(), "EngineECU");

    auto idxName = table->model()->index(0, Constants::Columns::MsgName);
    EXPECT_EQ(idxName.data().toString(), "SpeedMsg");
}

TEST_F(MessagesPageIntegrationTest, PopulatesSenderFilterOptionsFromParsedConfig)
{
    auto config =
        TestHelpers::DbcConfigBuilder()
            .node("EngineECU")
            .node("BrakeECU")
            .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg").transmitter("EngineECU"))
            .message(TestHelpers::DbcMessageBuilder(0x200, "BrakeMsg").transmitter("BrakeECU"))
            .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "options.dbc"));
    QApplication::processEvents();

    auto* filterBar = getFilterBar();
    ASSERT_NE(filterBar, nullptr);

    filterBar->setCurrentFilterText("BrakeECU");
    EXPECT_EQ(filterBar->currentFilterText(), "BrakeECU");
    EXPECT_EQ(filterBar->currentFilterData().toString(), "BrakeECU");

    filterBar->setCurrentFilterText("DoesNotExist");
    EXPECT_EQ(filterBar->currentFilterText(), Constants::MessagesPage::FilterAllText);
    EXPECT_TRUE(filterBar->currentFilterData().toString().isEmpty());
}

/**
 * @brief Test 4: Filter Reset at Config-change
 * Scenario:
 * 1. Load file A, set filter
 * 2. Load file B
 * 3. Check data
 */
TEST_F(MessagesPageIntegrationTest, ResetsFilterOnNewConfigLoad)
{
    // 1. Arrange: File A
    auto configA = TestHelpers::DbcConfigBuilder()
                       .node("Engine")
                       .message(TestHelpers::DbcMessageBuilder(0x100, "MsgA").transmitter("Engine"))
                       .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(configA, "A.dbc"));

    // Set Filters
    Core::StyledFilterBar* filterBar = getFilterBar();
    filterBar->setSearchText("MsgA");
    filterBar->setCurrentFilterText("Engine");
    QApplication::processEvents();

    QTableView* table = getMasterTable();
    ASSERT_EQ(table->model()->rowCount(), 1);

    // 2. Act: Load File B
    auto configB = TestHelpers::DbcConfigBuilder()
                       .node("Brakes")
                       .message(TestHelpers::DbcMessageBuilder(0x200, "MsgB").transmitter("Brakes"))
                       .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(configB, "B.dbc"));

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 1);

    auto idxSender = table->model()->index(0, Constants::Columns::MsgSender);
    EXPECT_EQ(idxSender.data().toString(), "Brakes");
}

/**
 * @brief Test 5: Master-Detail selection
 * Scenario: Click on a message in the table
 * Expectation:
 *  1. DetailView appears (if hidden).
 *  2. DetailView Header shows name of the message.
 *  3. DetailView signal-list shows signals of selected message.
 */
TEST_F(MessagesPageIntegrationTest, SelectionUpdatesDetailView)
{
    // 1. Arrange: Message "MsgWithSignals" with 2 signals
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x300, "MsgWithSignals")
                                   .signal(TestHelpers::DbcSignalBuilder("SigAlpha"))
                                   .signal(TestHelpers::DbcSignalBuilder("SigBeta")))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    QTableView* masterTable = getMasterTable();
    MessageDetailView* detailView = getDetailView();
    ASSERT_NE(masterTable, nullptr);
    ASSERT_NE(detailView, nullptr);

    // Initial: detail-view hidden
    ASSERT_TRUE(detailView->isHidden()) << "DetailView should be initially hidden";

    // 2. Act: Select first row
    QModelIndex firstRow = masterTable->model()->index(0, 0);
    masterTable->selectionModel()->select(firstRow,
                                          QItemSelectionModel::Select | QItemSelectionModel::Rows);
    masterTable->selectionModel()->setCurrentIndex(firstRow, QItemSelectionModel::SelectCurrent);
    QApplication::processEvents();

    // 3. Assert: Detail View Content
    // A) Check header
    auto* card = detailView->findChild<Core::CardWidget*>("card");
    ASSERT_NE(card, nullptr);
    EXPECT_TRUE(card->getTitle()->text().contains("MsgWithSignals"));

    // B) Check signal list
    QListView* signalList = detailView->getSignalList();
    ASSERT_NE(signalList, nullptr);

    QAbstractItemModel* model = signalList->model();
    QModelIndex root = signalList->rootIndex();

    // Validate model
    ASSERT_NE(model, nullptr);
    EXPECT_TRUE(root.isValid());

    // Amount of signals
    EXPECT_EQ(model->rowCount(root), 2);

    // Check first signal
    QModelIndex sigIdx = model->index(0, Constants::Columns::SigName, root);
    EXPECT_EQ(sigIdx.data().toString(), "SigAlpha");
}

/**
 * @brief Test 5: Deselection via click in empty space
 * Scenario: Click in empty space
 * Expectation: DetailView hidden
 */
TEST_F(MessagesPageIntegrationTest, DeselectionHidesDetailView)
{
    // 1. Arrange
    auto config =
        TestHelpers::DbcConfigBuilder().message(TestHelpers::DbcMessageBuilder(0x100, "A")).build();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QApplication::processEvents();

    QTableView* table = getMasterTable();
    MessageDetailView* detailView = getDetailView();

    // Select
    QModelIndex idx = table->model()->index(0, 0);
    table->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
    QApplication::processEvents();

    EXPECT_TRUE(!detailView->isHidden());

    // 2. Act: Deselection
    table->selectionModel()->clearSelection();
    table->selectionModel()->setCurrentIndex(QModelIndex(),
                                             QItemSelectionModel::SelectCurrent);  // Invalid Index
    QApplication::processEvents();

    // 3. Assert
    EXPECT_FALSE(!detailView->isHidden()) << "DetailView should be hidden after deselection";
}

/**
 * @brief Test 6: Toggle Selection (Click on already selected message)
 * Scenario: Click auf Row 0 (selected) -> Click auf Row 0 (deselected).
 */
TEST_F(MessagesPageIntegrationTest, ClickingSelectedRowDeselectsIt)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "ClickMe"))
                      .build();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getMasterTable();
    QAbstractItemModel* model = table->model();
    MessageDetailView* detailView = getDetailView();

    // First Selection
    QModelIndex idx = model->index(0, 0);
    table->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    table->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);

    ASSERT_TRUE(table->selectionModel()->isSelected(idx));
    ASSERT_FALSE(detailView->isHidden());

    // Simulate click on already selected row
    QRect rowRect = table->visualRect(idx);
    QPoint clickPoint = rowRect.center();
    // create MouseButtonPress event
    auto* viewport = table->viewport();
    QMouseEvent event(QEvent::MouseButtonPress, clickPoint, Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);

    // Send event to application
    QApplication::sendEvent(viewport, &event);

    // 3. Assert
    EXPECT_FALSE(table->selectionModel()->isSelected(idx))
        << "Row should be deselected after second selection";

    EXPECT_FALSE(table->selectionModel()->hasSelection()) << "Nothing should be selected";

    EXPECT_TRUE(detailView->isHidden());
}
/**
 * @brief Test 7: Empty State (No messages in model)
 * Scenario: A valid DBC-File without messages is loaded.
 * Expectation: Table is hidden, Empty label is shown
 */
TEST_F(MessagesPageIntegrationTest, ShowsEmptyStateOnNoData)
{
    // 1. Arrange: Build empty config
    auto config = TestHelpers::DbcConfigBuilder().build();

    // 2. Act:
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "empty.dbc"));

    QTableView* table = getMasterTable();

    QLabel* emptyLabel = nullptr;
    auto labels = messagesPage->findChildren<QLabel*>();
    for (auto* lbl : labels)
    {
        if (lbl->text() == Constants::MessagesPage::EmptyLabelText)
        {
            emptyLabel = lbl;
            break;
        }
    }

    // 3. Assert
    ASSERT_NE(table, nullptr);
    ASSERT_NE(emptyLabel, nullptr) << "Empty Label Widget not found";

    // Model has to be empty
    EXPECT_EQ(table->model()->rowCount(), 0);

    // Label has to be visible
    EXPECT_FALSE(emptyLabel->isHidden()) << "Empty Label should be visible when model is empty";

    // Table has to be hidden
    EXPECT_TRUE(table->isHidden()) << "Table should be hidden when model is empty";
}
/**
 * @brief Test 8: Empty State of master table at invalid search
 * Scenario: Message "Speed" exists, search for "Banana".
 */
TEST_F(MessagesPageIntegrationTest, ShowsEmptyLabelOnNoSearchResults)
{
    // 1. Arrange
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x100, "SpeedMsg"))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* table = getMasterTable();
    // Search empty label
    QLabel* emptyLabel = nullptr;
    auto labels = messagesPage->findChildren<QLabel*>();
    for (auto* lbl : labels)
    {
        if (lbl->text() == Constants::MessagesPage::EmptyLabelText)
        {
            emptyLabel = lbl;
            break;
        }
    }
    ASSERT_NE(emptyLabel, nullptr);
    Core::StyledFilterBar* filterBar = getFilterBar();

    // 2. Act: Search for nonsense
    filterBar->setSearchText("Banana");

    // 3. Assert
    EXPECT_EQ(table->model()->rowCount(), 0);
    EXPECT_FALSE(emptyLabel->isHidden()) << "Empty Label should be visible after";
    EXPECT_TRUE(table->isHidden());
}

/**
 * @brief Test 9: Detail View Empty State (Message without signals)
 * Scenario: Message "EmptyMsg" has 0 signals and is selected
 * Expectation: Detail View shows "No Signals defined" (Index 1 in stack)
 */
TEST_F(MessagesPageIntegrationTest, ShowsNoSignalsLabelForEmptyMessage)
{
    // 1. Arrange: Message without signals
    auto config = TestHelpers::DbcConfigBuilder()
                      .message(TestHelpers::DbcMessageBuilder(0x999, "EmptyMsg"))
                      .build();

    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));

    QTableView* masterTable = getMasterTable();
    MessageDetailView* detailView = getDetailView();

    auto* stack = detailView->findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr) << "QStackedWidget in DetailView not found";

    // 2. Act: Select message
    QModelIndex firstRow = masterTable->model()->index(0, 0);
    masterTable->selectionModel()->select(firstRow,
                                          QItemSelectionModel::Select | QItemSelectionModel::Rows);
    masterTable->selectionModel()->setCurrentIndex(firstRow, QItemSelectionModel::SelectCurrent);

    // 3. Assert
    // A) DetailView has to be visible
    EXPECT_FALSE(detailView->isHidden());

    // B) StackedWidget has to be on index 1 (empty state)
    EXPECT_EQ(stack->currentIndex(), 1) << "StackedWidget should show "
                                           "No Signals defined"
                                           "";

    // C) Check text
    auto* currentWidget = stack->currentWidget();
    auto* label = qobject_cast<QLabel*>(currentWidget);
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->text(), Constants::MessagesPage::NoSignalsIndicator);
}