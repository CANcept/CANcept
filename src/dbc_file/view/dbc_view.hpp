//
// Created by Adrian Rupp on 28.12.25.
//
#pragma once

#include <QStackedWidget>
#include <QWidget>
#include <memory>

#include "pages/ecus_page.hpp"
#include "pages/load_page.hpp"
#include "pages/messages_page.hpp"
#include "pages/overview_page.hpp"
#include "pages/signals_page.hpp"
#include "proxies.hpp"

namespace DbcFile {

/**
 * @class DbcView
 * @brief The main container for the DBC Tab (Composite View).
 *
 * @details
 * RESPONSIBILITIES:
 * 1. Layout: Manages the Sidebar navigation and the central Page Stack
 *    (Load, Overview, ECUs, Messages, Signals).
 * 2. Data Wiring: Owns the Proxy Models, connects them to the Source Model,
 *    and injects them into the specific Pages.
 * 3. Interaction Logic: Connects Search Bars, Filter Combos, and Selection Signals
 *    from the Pages to the corresponding Proxies.
 */
class DbcView : public QWidget
{
    Q_OBJECT

   public:
    explicit DbcView(QWidget* parent = nullptr);
    ~DbcView() override;

    /**
     * @brief Getter for the m_loadPage
     * @return Pointer to m_loadPage
     */
    [[nodiscard]] auto getLoadPage() const -> LoadPage&;

    /**
     * @brief Initializes the view with the source data.
     * @details
     * 1. Instantiates all Proxy Models.
     * 2. Injects the source model into proxies.
     * 3. Calls `setModel()` on all Sub-Pages using the correct proxies.
     * 4. Sets up internal signal-slot connections for filtering.
     *
     * @param model The raw DbcModel containing the data tree.
     */
    void setSourceModel(QAbstractItemModel* model);

    /**
     * @brief Sets the formatting delegate for tables.
     * @details
     * Applies the `DbcDelegate` to the Messages Master Table and the Signals Table
     * to ensure Hex values and Units are displayed correctly.
     */
    void setDataItemDelegate(QAbstractItemDelegate* delegate);

    /**
     * @brief Unlocks navigation (Sidebar) after a successful file load.
     * @details
     * Initially, only the LoadPage is accessible. Once data is loaded,
     * this enables the sidebar so the user can switch views.
     */
    void setNavigationEnabled(bool enabled) const;

   signals:
    /**
     * @brief Forwarded signal from LoadPage.
     *
     * @details Tells the DbcComponent that the user has selected a file to parse
     * (via Drag & Drop or File Dialog).
     *
     * @param filePath The absolute path to the file.
     */
    void fileLoadRequested(const QString& filePath);

   private slots:
    /**
     * @brief Handles sidebar navigation to switch the active page in the stack.
     * @details Connects to QListView::clicked signal from the Sidebar and handles switching to the
     * page in m_contentStack that corresponds to the given index.
     * Also handles reset to default style of the m_loadPage UI when switching back to m_loadPage
     * from another page.
     */
    void onSidebarSelectionChanged(const QModelIndex& index);

    // --- ECU PAGE INTERACTION ---

    /**
     * @brief Updates the tree proxy search filter.
     * @caller EcusPage (search bar).
     */
    void onEcuFilterTextChanged(const QString& text);

    /**
     * @brief Updates the tree proxy category filter.
     * @caller EcusPage (combo box).
     */
    void onEcuFilterTypeChanged(int index);

    // --- MESSAGES PAGE INTERACTION ---

    /**
     * @brief Updates the messages flat list proxy search filter.
     * @caller MessagesPage (master search bar).
     */
    void onMessageFilterTextChanged(const QString& text);

    /**
     * @brief Updates the messages flat list proxy category filter.
     * @caller MessagesPage (master combo box).
     */
    void onMessageFilterTypeChanged(int index);

    /**
     * @brief Master-Detail Logic for Messages Page.
     *
     * @caller MessagesPage (selection changed signal).
     *
     * @details
     * 1. Maps the proxy index (List) to the source index (Tree).
     * 2. Updates the `m_messageDetailProxy` to show children of this message.
     * 3. Updates the Title in the Detail View.
     * 4. Shows the Detail Pane.
     */
    void onMessageSelected(const QModelIndex& proxyIndex);

    // --- SIGNALS PAGE INTERACTION ---

    /**
     * @brief Updates the all-signals proxy search filter.
     * @caller SignalsPage (search bar).
     */
    void onSignalFilterTextChanged(const QString& text);

    /**
     * @brief Updates the all-signals proxy category filter (e.g. Unit).
     * @caller SignalsPage (combo box).
     */
    void onSignalFilterTypeChanged(int index);

   private:
    /**
     * @brief Describes a single entry in the sidebar model.
     *
     * @details
     * This structure defines the data required to create one sidebar item,
     * including its icon, display title, and initial enabled state. It is
     * used to populate the sidebar model in a structured and maintainable
     * way.
     */
    struct SidebarEntry {
        QString iconPath;
        QString title;
        bool enabled;
    };

    /**
     * @brief Prevents deselection of items in the sidebar list.
     *
     * @details
     * This method ensures that the selection in the sidebar list (`m_sidebarList`) cannot
     * be completely cleared when the user clicks on an empty area. It connects a lambda
     * to the `selectionChanged` signal of the `QItemSelectionModel`. If the new selection
     * is empty (e.g., a click on an empty space), the previously selected items are
     * reselected so that at least one item remains selected at all times.
     */
    void disableSidebarDeselection();

    /**
     * @brief Initializes and configures the sidebar list view.
     *
     * @details
     * This method sets up the `m_sidebarList` as a `QListView` with specific visual and
     * interaction properties. It configures the list for single selection of rows, disables
     * editing and selection rectangle visibility, sets a maximum width, and removes the frame.
     *
     * Additionally, it applies a stylesheet based on the application's theme, defining
     * background colors, borders, text colors, font size, item padding/margins, border
     * radius, and selection highlighting. The colors and spacing values are retrieved from
     * the `Core::ThemeManager` to ensure consistency with the overall application theme.
     */
    void setupSidebarList();

    /**
     * @brief Sets up the model for the sidebar list.
     *
     * @details
     * This method creates a `QStandardItemModel` for `m_sidebarList` and populates it
     * with predefined sidebar items. The items include:
     *   1. "Load New" – always enabled and initially selected.
     *   2. "Overview" – initially disabled.
     *   3. "ECUs" – initially disabled.
     *   4. "Messages" – initially disabled.
     *   5. "Signals" – initially disabled.
     *
     * Each item is created with a corresponding icon and title from the `Constants::Sidebar`
     * namespace. After all items are added, the model is set on `m_sidebarList`, deselection of
     * sidebar items is disabled to ensure a persistent selection and the
     * first item ("Load New") is selected by default.
     */
    void setSidebarModel();

    /**
     * @brief Initializes and assembles the user interface of the DBC view.
     *
     * @details
     * This method builds the main UI layout for the DBC view. It creates the main horizontal
     * layout, and sets up the sidebar and content area.
     *
     * The sidebar is initialized, populated with its model, assigned a custom item
     * delegate, and added to the main layout.
     *
     * Additionally, a `QStackedWidget` is created to hold the different content pages.
     * Subviews are created and signal-slot connections are established to complete
     * the UI setup.
     */
    void setupUi();

    /**
     * @brief Helper to instantiate all Page Widgets.
     * @caller setupUi().
     */
    void createSubViews();

    /**
     * @brief Helper to connect Page signals (Search/Select) to View slots.
     * @caller setSourceModel().
     */
    void setupConnections();

    // --- UI Structure ---
    QListView* m_sidebarList;
    QStackedWidget* m_contentStack;

    // --- Pages ---
    // (Raw pointers because Qt manages their memory via parent/layout)
    LoadPage* m_loadPage;
    OverviewPage* m_overviewPage;
    EcusPage* m_ecuPage;
    MessagesPage* m_messagesPage;
    SignalsPage* m_signalsPage;

    // --- Data Proxies (Owned by the View) ---

    // /**
    //  * @brief Hierarchy filter for the ECU Page.
    //  * Maintains tree structure (Root->ECU->Message->Signal).
    //  */
    // std::unique_ptr<TreeFilterProxy> m_ecuTreeProxy;
    //
    // /**
    //  * @brief Flat list of ECUs for the Overview Page (Tiles).
    //  * Flattens the tree to just show ECU nodes.
    //  */
    // std::unique_ptr<FlatListProxy> m_ecuOverviewProxy;
    //
    // /**
    //  * @brief Flat list of Messages.
    //  * SHARED by:
    //  * 1. Overview Page (Messages Tile List)
    //  * 2. Messages Page (Master Table)
    //  */
    // std::unique_ptr<FlatListProxy> m_messagesProxy;
    //
    // /**
    //  * @brief Detail filter for Messages Page.
    //  * Shows only signals of the currently selected message.
    //  */
    // std::unique_ptr<SingleMessageProxy> m_messageDetailProxy;
    //
    // /**
    //  * @brief Flat list of ALL Signals in the system.
    //  * Used for the Signals Page table.
    //  */
    // std::unique_ptr<FlatListProxy> m_allSignalsProxy;
};

}  // namespace DbcFile