#pragma once
#include <QLabel>
#include <QListView>
#include <QSplitter>
#include <QWidget>

#include "../../../core/widgets/common/searchable_filter_widgets.hpp"
namespace DbcFile {
// ==============================================================================
// Message Detail View (Bottom Pane)
// ==============================================================================

/**
 * @class MessageDetailView
 * @brief The lower half of the Message page showing details (signals).
 *
 * @details
 * **VISUALS:**
 * - Header Label (Message Name).
 * - List of Signals (Cards).
 *
 * **LOGIC:**
 * Updates content based on selection in the Master table.
 */
class MessageDetailView : public QWidget
{
    Q_OBJECT
   public:
    explicit MessageDetailView(QWidget* parent = nullptr);

    /**
     * @brief Returns the list view (for the signals).
     * @caller MessagesPage::setDetailModel().
     * @return Pointer to the internal signal list view.
     */
    [[nodiscard]] auto getSignalList() const -> QListView*;

    /**
     * @brief Sets the title text (Message Name).
     * @caller MessagesPage::setDetailTitle().
     * @param title The name of the selected message.
     */
    void setMessageTitle(const QString& title);

   private:
    /**
     * @brief Initializes the detail view layout.
     * @caller Constructor.
     * @details Creates the Title Label and the Signal List View (configured with flow layout).
     */
    void setupUi();

    QLabel* m_lblTitle;
    QListView* m_signalList;
};

// ==============================================================================
// Messages Page (Master-Detail)
// ==============================================================================

/**
 * @class MessagesPage
 * @brief The main page for the Messages Tab (SRS 6.4).
 *
 * @details
 * **VISUALS:**
 * Master-Detail view using a vertical splitter.
 *
 * **LOGIC:**
 * - Displays a Master List of all messages (top).
 * - Emits `messageSelectionChanged` when the user selects a row.
 * - Receives updates from DbcView via `setDetailTitle` to update the bottom pane.
 */
class MessagesPage : public QWidget
{
    Q_OBJECT

   public:
    explicit MessagesPage(QWidget* parent = nullptr);
    ~MessagesPage() override = default;

    /**
     * @brief Sets the model for the master table (Top).
     * @caller DbcView::setSourceModel().
     * @details Connects to the selection model to detect row changes.
     */
    void setMasterModel(QAbstractItemModel* model);

    /**
     * @brief Sets the model for the detail view (Bottom).
     * @caller DbcView::setSourceModel().
     * @details This connects the model to the Signal List in the Detail View.
     */
    void setDetailModel(QAbstractItemModel* model);

    /**
     * @brief Sets the delegate for the signal list (Card rendering).
     * @caller DbcView::setDataItemDelegate().
     */
    void setSignalDelegate(QAbstractItemDelegate* delegate);

    /**
     * @brief Toggles the visibility of the bottom detail pane.
     * @caller DbcView::onMessageSelected().
     */
    void showDetailsPane(bool visible);

    /**
     * @brief Access to the filter combo box of the master table.
     * @caller DbcView::createSubViews().
     */
    [[nodiscard]] auto getMasterFilterCombo() const -> QComboBox*;

    /**
     * @brief Pass-through method to set the title in the detail view.
     * @caller DbcView::onMessageSelected().
     */
    void setDetailTitle(const QString& title);

   signals:
    /**
     * @brief Emitted when the user selects a message in the master table.
     * @caller Internal slot `onSelectionChanged`.
     * @param proxyIndex The index in the Proxy Model corresponding to the selected message.
     */
    void messageSelectionChanged(const QModelIndex& proxyIndex);

    /**
     * @brief Emitted when the search text changes.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void masterFilterTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter dropdown changes.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void masterFilterTypeChanged(int index);

   private slots:
    /**
     * @brief Internal slot connected to the TableView's selection model.
     * @caller Qt ItemSelectionModel.
     */
    void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

   private:
    /**
     * @brief Assembles the master-detail layout.
     * @caller Constructor.
     * @details Creates the Splitter, SearchableFilterTable (Master), and MessageDetailView
     * (Detail).
     */
    void setupUi();

    QSplitter* m_splitter;
    Core::SearchableFilterTable* m_messagesTable;  // Master (Top)
    MessageDetailView* m_detailView;               // Detail (Bottom)
};
}  // namespace DbcFile
