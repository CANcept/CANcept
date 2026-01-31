//
// Created by Adrian Rupp on 28.12.25.
//
#pragma once

#include <QStackedWidget>
#include <QWidget>
#include <memory>

#include "core/widgets/sidebar.hpp"
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
    /**
     * @brief Constructs the DbcView widget.
     *
     * Initializes the user interface and sets up all child widgets.
     *
     * @param parent Optional parent widget.
     */
    explicit DbcView(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~DbcView() override;

    /**
     * @brief Getter for the m_loadPage.
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
     * @brief Enables or disables sidebar navigation.
     *
     * Forwards the state to the sidebar widget.
     *
     * @param enabled Whether navigation should be enabled.
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
     * @brief Handles sidebar tab selection changes.
     *
     * Currently, ignores double clicks on one item.
     * Switches the visible page in the content stack and performs
     * cleanup actions when leaving certain pages.
     *
     * @param index Index of the selected sidebar tab.
     */
    void onSidebarSelectionChanged(int index);

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
     * @brief Sets up the main user interface layout.
     *
     * Creates the sidebar, content stack, and initializes
     * all subviews.
     */
    void setupUi();

    /**
     * @brief Creates and registers all subviews.
     *
     * The order of added widgets must match the sidebar
     * tab order.
     */
    void createSubViews();

    /**
     * @brief Sets up signal-slot connections.
     *
     * Connects navigation and page-level signals to the
     * appropriate handlers and external signals.
     */
    void setupConnections();

    // --- UI Structure ---
    Core::Sidebar* m_sidebar;
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