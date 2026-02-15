#pragma once
#include <QListView>
#include <QSplitter>
#include <QStackedWidget>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
// =============================================================================
// 1. MessageDetailView
// =============================================================================
namespace DbcFile {
/**
 * @class MessageDetailView
 * @brief Displays detailed information about a selected CAN message.
 *
 * This widget represents the lower part of the MessagesPage (detail pane).
 * It consists of:
 * - A header section (title + subtitle inside a CardWidget)
 * - A QListView showing all signal children of the selected message
 * - A stacked widget to show a "No Signals" message if the message has no signals
 *
 * The list view operates by setting a root index in the source model,
 * which allows displaying only the direct signal children of the message.
 */
class MessageDetailView : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the detail view UI.
     * @param parent Optional parent widget.
     */
    explicit MessageDetailView(QWidget* parent = nullptr);

    /**
     * @brief Returns the internal signal list view.
     *
     * Used by the page to assign models and delegates.
     */
    [[nodiscard]] auto getSignalList() const -> QListView*;

    /**
     * @brief Sets the root index of the signal list.
     * @param index Source model index of the selected message.
     *
     * The list view will display only the direct children (signals)
     * of this index. If the message has no signals, a placeholder
     * "No Signals" label is shown instead.
     */
    void setRootIndex(const QModelIndex& index);

    /**
     * @brief Applies the style from the stylesheet.
     *
     * Updates the detail label's stylesheet from Style::MessagesPage.
     */
    void applyStyle();

    /**
     * @brief Updates the header metadata for the selected message.
     *
     * @param name   Message name.
     * @param id     Message identifier.
     * @param sender Sender ECU name.
     * @param dlc    Data length code.
     */
    void updateHeaderInfo(const QString& name, uint id, const QString& sender, int dlc);

   private:
    /**
     * @brief Sets up the UI elements of the detail view.
     *
     * Creates the card widget, stacked widget, signal list, "no signals" label,
     * and adds them to the layout.
     */
    void setupUi();

    /**
     * @brief Handles custom events for styling.
     *
     * Refreshes styles if a Core::StyleEvent is received.
     * @param event The event object.
     * @return True if the event was handled.
     */
    bool event(QEvent* event) override;

   private:
    Core::CardWidget* m_card = nullptr; /**< Card widget containing the detail header */
    QListView* m_signalList = nullptr; /**< List view showing the signals of the selected message */
    QStackedWidget* m_stack =
        nullptr; /**< Stack to switch between signal list and "No Signals" label */
    QLabel* m_detailLabel = nullptr; /**< Label displayed when there are no signals */
};

// =============================================================================
// 2. MessagesPage
// =============================================================================

/**
 * @class MessagesPage
 * @brief Main page of the Messages tab.
 *
 * Implements a vertical master-detail layout using QSplitter:
 *
 * Top section:
 *   - Searchable and filterable table of messages.
 *
 * Bottom section:
 *   - MessageDetailView displaying the selected message and its signals.
 *
 * Behavior:
 * - Clicking a row selects it and shows the detail card.
 * - Clicking the same row again deselects it and hides the detail card.
 * - Clicking outside the table deselects the current selection.
 * - Messages with no signals show a "No Signals" placeholder.
 */
class MessagesPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the Messages page UI.
     * @param parent Optional parent widget.
     */
    explicit MessagesPage(QWidget* parent = nullptr);

    ~MessagesPage() override = default;

    /**
     * @brief Selects a message for detail display.
     * @param index Source model index of the message.
     */
    void selectMessageIndex(const QModelIndex& index);

    /**
     * @brief Sets the model for the master table (top section).
     *
     * Typically, a FlatListProxy showing messages.
     */
    void setMasterModel(QAbstractItemModel* model);

    /**
     * @brief Configures column visibility and resize behavior of the master table.
     */
    void configureMasterColumns(QTableView* table, const QAbstractItemModel* model);

    /**
     * @brief Sets the model for the detail view (bottom section).
     *
     * Usually the full tree model so the list can use root indices.
     */
    void setDetailModel(QAbstractItemModel* model);

    /**
     * @brief Populates the sender filter combo box.
     */
    auto setAvailableSenders(const QStringList& senders) -> void;

    /**
     * @brief Handles mouse presses outside the table to clear selection.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Event filter for clicks inside the table viewport.
     *
     * Used to implement toggle deselection when clicking the same row
     * or clicks in the empty space below rows.
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * @brief Handles custom events for styling.
     *
     * Refreshes style for child widgets if a Core::StyleEvent is received.
     * @param event The event object.
     * @return True if the event was handled.
     */
    bool event(QEvent* event) override;

    /**
     * @brief Updates the empty state label visibility depending on tree contents.
     */
    void updateEmptyState();

   signals:
    /**
     * @brief Emitted when the user selects a message in the master table.
     */
    void messageSelectionChanged(const QModelIndex& proxyIndex);

    /**
     * @brief Forwarded search text change from the filter table.
     */
    void masterFilterTextChanged(const QString& text);

    /**
     * @brief Emitted when the sender filter changes.
     */
    void filterSenderChanged(const QString& sender);

   private slots:
    /**
     * @brief Handles selection changes in the master table.
     *
     * Shows or hides the detail card depending on the selection.
     */
    void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

    /**
     * @brief Handles changes in the sender filter combo box.
     */
    void onFilterIndexChanged(int index);

   private:
    /**
     * @brief Sets up the UI for the MessagesPage.
     *
     * Creates the master/detail layout with table card on top
     * and detail view at the bottom.
     */
    void setupUi();

    /**
     * @brief Configures the master messages table.
     *
     * Sets the item delegate, selection behavior, and search placeholder.
     */
    void configureMasterTable();

    /**
     * @brief Applies style updates to child widgets.
     *
     * Propagates style refresh to messages table and detail view.
     */
    void applyStyle();

   private:
    QSplitter* m_splitter = nullptr; /**< Splitter dividing master and detail panes */
    Core::SearchableFilterTable* m_messagesTable = nullptr; /**< Table widget with search/filter */
    MessageDetailView* m_detailView = nullptr; /**< Detail view of the selected message */
    QLabel* m_emptyLabel = nullptr;            /**< Label shown when no messages are found; */
};
}  // namespace DbcFile