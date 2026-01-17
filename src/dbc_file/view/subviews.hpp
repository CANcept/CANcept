//
// Created by Adrian Rupp on 28.12.25.
//
#pragma once

#include <QDataWidgetMapper>
#include <QFrame>
#include <QLabel>
#include <QListView>
#include <QSplitter>
#include <QWidget>

// Include generic widgets (SearchableFilterTable, SearchableFilterTree)
#include "searchable_filter_widgets.hpp"

// Forward Declarations
class QDragEnterEvent;
class QDropEvent;

namespace DbcFile {

// ==============================================================================
// 1. Load Page
// ==============================================================================

/**
 * @class LoadPage
 * @brief The landing page for the DBC module (SRS 6.1 "Load New").
 *
 * @details
 * **VISUALS:**
 * Provides a centered "Card" layout.
 *
 * **LOGIC:**
 * Allows uploading a DBC file via Drag & Drop or by clicking to open a file dialog.
 */
class LoadPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the LoadPage widget.
     * @caller DbcView::createSubViews().
     * @param parent The parent widget.
     */
    explicit LoadPage(QWidget* parent = nullptr);
    ~LoadPage() override = default;

   signals:
    /**
     * @brief Emitted when a file is successfully selected by the user.
     *
     * @details Triggered by either dropping a valid file onto the widget or selecting
     * one via the system file dialog.
     *
     * @param filePath The absolute path to the selected file.
     */
    void fileSelected(const QString& filePath);

   protected:
    /**
     * @brief Handles drag enter events to validate dropped data.
     * @caller Qt Event Loop (when dragging over widget).
     * @details Updates the UI (e.g., green border) if the dragged item is a dbc file.
     */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @brief Handles drop events to extract the file path.
     * @caller Qt Event Loop (when mouse released).
     */
    void dropEvent(QDropEvent* event) override;

    /**
     * @brief Event Filter for the upload Box.
     * @caller Qt Event Loop (before event reaches target).
     * @details Used to detect clicks on the central "Upload Box" frame to trigger the file dialog.
     */
    auto eventFilter(QObject* watched, QEvent* event) -> bool override;

   private slots:
    /**
     * @brief Opens a QFileDialog to let the user browse for a DBC file.
     * @caller Internal (on click event).
     */
    void onBrowseButtonClicked();

   private:
    /**
     * @brief Initializes the visual components and layout.
     * @caller Constructor.
     * @details Creates the central card frame, the upload box, and applies the stylesheets.
     */
    void setupUi();

    /** @brief The clickable area for file upload. */
    QFrame* m_uploadBoxFrame;
};

// ==============================================================================
// 2. Overview Page
// ==============================================================================

/**
 * @class OverviewPage
 * @brief The Overview page displaying file statistics and summaries (SRS 6.2).
 *
 * @details
 * **VISUALS:**
 * - Section 1: File Metadata (Version, Date).
 * - Section 2: KPI Cards (Counts for ECUs, Messages, Signals).
 * - Section 3: Horizontal flow lists for ECUs and Messages.
 *
 * **LOGIC:**
 * Uses QDataWidgetMapper for Sections 1 & 2, and QListViews for Section 3.
 */
class OverviewPage : public QWidget
{
    Q_OBJECT

   public:
    explicit OverviewPage(QWidget* parent = nullptr);
    ~OverviewPage() override = default;

    /**
     * @brief Returns the mapper used to populate the static labels (Top/Middle sections).
     * @caller DbcView::setSourceModel() to inject data.
     * @return Pointer to the internal mapper.
     */
    [[nodiscard]] auto getMapper() const -> QDataWidgetMapper*
    {
        return m_mapper;
    }

    /**
     * @brief Returns the list view for the ECUs section.
     * @caller DbcView::setSourceModel() to set the ECU Proxy.
     * @return Pointer to the ECU list view.
     */
    [[nodiscard]] auto getEcuList() const -> QListView*
    {
        return m_ecuList;
    }

    /**
     * @brief Returns the list view for the Messages section.
     * @caller DbcView::setSourceModel() to set the Message Proxy.
     * @return Pointer to the Message list view.
     */
    [[nodiscard]] auto getMessageList() const -> QListView*
    {
        return m_messageList;
    }

   private:
    /**
     * @brief Assembles the dashboard layout.
     * @caller Constructor.
     * @details Creates the ScrollArea, File Info Group, Stat Cards Layout, and the Overview Lists.
     */
    void setupUi();

    /**
     * @brief Helper to create one of the 4 Statistics cards in the middle section.
     * @caller Internal setupUi().
     */
    auto createStatCard(const QString& title, QLabel*& valueLabelPtr,
                        const QString& iconName) -> QWidget*;

    QDataWidgetMapper* m_mapper;

    // Targets for m_mapper
    QLabel* m_lblVersion;
    QLabel* m_lblLoadedDate;
    QLabel* m_lblEcuCount;
    QLabel* m_lblMessageCount;
    QLabel* m_lblSignalCount;
    QLabel* m_lblLargestMessageName;

    // List Views
    QListView* m_ecuList;
    QListView* m_messageList;
};

// ==============================================================================
// 3. ECUs Page
// ==============================================================================

/**
 * @class EcusPage
 * @brief The page displaying the ECU hierarchy.
 *
 * @details
 * **VISUALS:**
 * - Uses SearchableFilterTree (Search + Filter + Tree).
 * - Standard Qt Expansion (Arrow on the left).
 * - Custom Delegate (EcuTreeDelegate) used for Card-like rendering.
 *
 * **LOGIC:**
 * Acts as a wrapper around the tree view and forwards user input signals.
 */
class EcusPage : public QWidget
{
    Q_OBJECT
   public:
    explicit EcusPage(QWidget* parent = nullptr);
    ~EcusPage() override = default;

    /**
     * @brief Sets the source model for the internal tree view.
     * @caller DbcView::setSourceModel() to inject the TreeFilterProxy.
     * @param model The proxy model to be displayed.
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Access to the filter combo box to populate it with options.
     * @caller DbcView::createSubViews() to add items like "All ECUs".
     * @return Pointer to the internal QComboBox.
     */
    [[nodiscard]] auto getFilterCombo() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the user types text into the search bar.
     * @caller Internal SearchableFilterTree signal forwarding.
     * @param text The current text in the search field.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user selects a different option in the filter combo box.
     * @caller Internal SearchableFilterTree signal forwarding.
     * @param index The index of the selected combo box item.
     */
    void filterTypeChanged(int index);

   private:
    /**
     * @brief Initializes the page layout.
     * @caller Constructor.
     * @details Instantiates the SearchableFilterTree and installs the EcuTreeDelegate on the view.
     */
    void setupUi();

    SearchableFilterTree* m_treeWidget;
};

// ==============================================================================
// 4. Message Detail View (Bottom Pane)
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
// 5. Messages Page (Master-Detail)
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
    SearchableFilterTable* m_messagesTable;  // Master (Top)
    MessageDetailView* m_detailView;         // Detail (Bottom)
};

// ==============================================================================
// 6. Signals Page
// ==============================================================================

/**
 * @class SignalsPage
 * @brief The page displaying a flat list of signals (SRS 6.5).
 *
 * @details
 * **VISUALS:**
 * - Top: Search Bar + Filter ComboBox (e.g. "All Units").
 * - Body: A table listing the signals matching search and filter.
 *
 * **LOGIC:**
 * Wraps a SearchableFilterTable and forwards user interaction signals.
 */
class SignalsPage : public QWidget
{
    Q_OBJECT

   public:
    explicit SignalsPage(QWidget* parent = nullptr);
    ~SignalsPage() override = default;

    /**
     * @brief Sets the model for the table view.
     * @caller DbcView::setSourceModel().
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Access to the filter combo to populate unit filters (e.g. "rpm", "km/h").
     * @caller DbcView::createSubViews().
     */
    [[nodiscard]] auto getFilterCombo() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the user types in the search bar.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user changes the Unit filter dropdown.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void filterTypeChanged(int index);

   private:
    /**
     * @brief Initializes the page layout.
     * @caller Constructor.
     * @details Instantiates the SearchableFilterTable.
     */
    void setupUi();

    SearchableFilterTable* m_tableWidget;
};

}  // namespace DbcFile