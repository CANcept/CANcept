/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <memory>

#include "core/widgets/sidebar.hpp"
#include "pages/ecus_page.hpp"
#include "pages/load_page.hpp"
#include "pages/messages_page.hpp"
#include "pages/overview_page.hpp"
#include "pages/signals_page.hpp"
#include "proxies/ecu_tree_proxy.hpp"
#include "proxies/flat_list_proxy.hpp"

namespace DbcFile {

/**
 * @class DbcView
 * @brief Main view container for browsing and interacting with parsed DBC content.
 *
 * Responsibilities:
 * - Owns the sidebar navigation and the stacked page container.
 * - Creates and hosts all main pages (load, overview, ECUs, messages, signals).
 * - Binds a source model to page-specific proxy models.
 * - Routes UI interactions (navigation, filtering, selection) to the appropriate proxies/pages.
 *
 * Note:
 * Page widgets are owned by Qt via parent-child relationships. Proxy models are owned by
 * this view via std::unique_ptr.
 */
class DbcView final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the view and builds the UI.
     * @param parent Optional parent widget.
     */
    explicit DbcView(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~DbcView() override;

    [[nodiscard]] auto getOverviewEcuProxy() const -> FlatListProxy*
    {
        return m_ecuOverviewProxy.get();
    }
    [[nodiscard]] auto getOverviewMessageProxy() const -> FlatListProxy*
    {
        return m_messageOverviewProxy.get();
    }
    [[nodiscard]] auto getMessagesProxy() const -> FlatListProxy*
    {
        return m_messagesProxy.get();
    }
    [[nodiscard]] auto getSignalsProxy() const -> FlatListProxy*
    {
        return m_signalsProxy.get();
    }
    [[nodiscard]] auto getEcuTreeProxy() const -> EcuTreeProxy*
    {
        return m_ecuTreeProxy.get();
    }

    /**
     * @brief Returns the load page instance.
     * @return Reference to the internal LoadPage.
     */
    [[nodiscard]] auto getLoadPage() const -> LoadPage&;

    /**
     * @brief Sets the list of available signal units used for filtering.
     * @param units List of unit strings.
     */
    void setSignalUnits(const QStringList& units) const;

    /**
     * @brief Sets the list of available message senders used for filtering.
     * @param senders List of sender names.
     */
    void setAvailableSenders(const QStringList& senders) const;

    /**
     * @brief Injects a source model and wires all proxies/pages to it.
     * @param model Source model providing the DBC item hierarchy.
     *
     * This method:
     * - Resets all active filters (search text, categories) in both proxies and UI pages
     *   to ensure data from the new file is immediately visible.
     * - Creates the required proxy models (flat proxies and tree proxy).
     * - Assigns the source model to each proxy.
     * - Assigns proxies to the corresponding pages.
     * - Connects model reset events to UI updates where needed.
     */
    void setSourceModel(QAbstractItemModel* model);

    /**
     * @brief Enables or disables sidebar navigation.
     * @param enabled If false, navigation tabs should be disabled.
     */
    void setNavigationEnabled(bool enabled) const;

   signals:
    /**
     * @brief Emitted when the user requests loading a DBC file.
     * @param filePath Absolute path to the selected file.
     */
    void fileLoadRequested(const QString& filePath);

   private slots:
    /**
     * @brief Handles sidebar tab changes and switches the stacked page.
     * @param index Selected tab/page index.
     *
     * The handler ignores redundant selections and may perform page-specific cleanup
     * actions when leaving certain pages (e.g. resetting status on the load page).
     */
    void onSidebarSelectionChanged(int index) const;

    /**
     * @brief Updates the ECU tree proxy search filter text.
     * @param text Search string entered by the user.
     */
    void onEcuFilterTextChanged(const QString& text) const;

    /**
     * @brief Updates the ECU tree proxy filter category.
     * @param index Selected filter category index.
     */
    void onEcuFilterIndexChanged(int index) const;

    /**
     * @brief Updates the messages flat proxy search filter text.
     * @param text Search string entered by the user.
     */
    void onMessageFilterTextChanged(const QString& text);

    /**
     * @brief Updates the sender filter for the messages flat proxy.
     * @param sender Selected sender name.
     */
    void onMessageSenderChanged(const QString& sender);

    /**
     * @brief Handles selection changes in the messages master view.
     * @param proxyIndex Index in the messages proxy model.
     *
     * The index is mapped back to the source model and forwarded to the messages page
     * to update its detail panel.
     */
    void onMessageSelected(const QModelIndex& proxyIndex);

    /**
     * @brief Updates the signals flat proxy search filter text.
     * @param text Search string entered by the user.
     */
    void onSignalFilterTextChanged(const QString& text);

    /**
     * @brief Updates the unit filter for the signals flat proxy.
     * @param unit Selected unit name.
     */
    void onSignalUnitChanged(const QString& unit) const;

    /**
     * @brief Adds a page to the stacked content widget and a tab to the sidebar.
     * @param page Page widget to add.
     * @param title Sidebar tab title.
     * @param iconPath Resource path to the tab icon.
     * @param enabled Initial enabled state for the tab.
     */
    void addPage(QWidget* page, const QString& title, const QString& iconPath, bool enabled) const;

    /**
     * @brief Creates and registers all pages and sidebar tabs.
     *
     * The creation order defines the tab indices used for navigation.
     */
    void setupSidebarTabs();

   private:
    /**
     * @brief Builds the main layout (sidebar + stacked content) and creates pages.
     */
    void setupUi();

    /**
     * @brief Connects sidebar and page signals to view slots and outgoing signals.
     */
    void setupConnections();

   private:
    /** @brief Source model backing the DBC hierarchy. Not owned by this view. */
    QAbstractItemModel* m_model = nullptr;

    // --- UI Structure (Qt-owned via parent-child hierarchy) ---
    Core::Sidebar* m_sidebar = nullptr;
    QStackedWidget* m_contentStack = nullptr;

    // --- Pages (Qt-owned via parent-child hierarchy) ---
    LoadPage* m_loadPage = nullptr;
    OverviewPage* m_overviewPage = nullptr;
    EcusPage* m_ecuPage = nullptr;
    MessagesPage* m_messagesPage = nullptr;
    SignalsPage* m_signalsPage = nullptr;

    // --- Proxy Models (owned by this view) ---

    /** @brief Tree proxy used by the ECU page (preserves ECU->Message->Signal structure). */
    std::unique_ptr<EcuTreeProxy> m_ecuTreeProxy;

    /** @brief Flat list of ECUs used by the overview page (ECU tile/list). */
    std::unique_ptr<FlatListProxy> m_ecuOverviewProxy;

    /** @brief Flat list of messages used by the overview page (message tile/list). */
    std::unique_ptr<FlatListProxy> m_messageOverviewProxy;

    /** @brief Flat list of messages used as the master model on the messages page. */
    std::unique_ptr<FlatListProxy> m_messagesProxy;

    /** @brief Flat list of all signals used by the signals page table. */
    std::unique_ptr<FlatListProxy> m_signalsProxy;
};

}  // namespace DbcFile
