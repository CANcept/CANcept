//
// Created by Adrian Rupp on 28.01.26.
//
#pragma once
#include <QListView>

#include "sidebar_delegate.hpp"

class QStandardItem;
class QStandardItemModel;
namespace Core {

/**
 * @class Sidebar
 * @brief A reusable navigation sidebar widget based on QListView.
 *
 * @details
 * This class encapsulates the visual styling (CSS), item behavior (bold on selection, spacing),
 * and interaction logic (preventing empty selection, emitting signals on valid clicks)
 * required for a vertical navigation bar. It is designed to be populated via `addTab()`
 * and integrates with the application's ThemeManager.
 */
class Sidebar : public QListView {
    Q_OBJECT

public:
    /**
     * @brief Constructs the Sidebar widget.
     * @param parent The parent widget.
     */
    explicit Sidebar(QWidget* parent = nullptr);
    ~Sidebar() override = default;

void setToolTipText(const QString& toolTipText);
    /**
     * @brief Adds a new navigation tab to the sidebar.
     *
     * If this is the first tab being added, it will be selected automatically.
     *
     * @param icon Icon displayed next to the tab title.
     * @param title Text label of the tab.
     * @param enabled Whether the tab is enabled and selectable.
     */
    void addTab(const QIcon& icon, const QString& title, bool enabled = true);


    /**
     * @brief Enables or disables all tabs except the first one.
     *
     * @details
     * Used to lock navigation (e.g., when no file is loaded in the DBC File tab) or unlock it.
     * When locking, it automatically resets the selection to the first item.
     *
     * @param enabled True to unlock all tabs, False to lock them.
     */
    void setNavigationEnabled(bool enabled) const;

    signals:
    /**
     * @brief Emitted when a tab is selected by the user.
     *
     * The signal is only emitted for valid and enabled items.
     *
     * @param index Row index of the selected tab.
     */
    void tabSelected(int index);

private:
    /**
     * @brief Initializes visual appearance and view configuration.
     *
     * Applies theme-based styling, configures selection behavior,
     * and sets up the item model and delegate.
     */
    void setupUi();


    /**
     * @brief Sets up internal signal-slot connections.
     *
     * Handles user interaction such as clicks and ensures that the
     * selection can never become empty.
     */
    void setupConnections();

    QStandardItemModel* m_model;
    Core::SidebarDelegate* m_delegate;
};

}
