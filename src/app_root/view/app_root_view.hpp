#pragma once

#include <QListView>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QWidget>

#include "app_root/delegate/app_root_delegate.hpp"
#include "app_root/model/app_root_model.hpp"
#include "core/interface/i_tab_component.hpp"

namespace AppRoot {

/**
 * @class AppRootView
 * @brief The main UI Shell acting as the Window.
 * @details
 * implements the Strict MVD pattern.
 * - Tabs: Rendered by QListView (via AppRootDelegate).
 * - Content: Rendered by QStackedWidget.
 * The View acts as a mediator, syncing the SelectionModel of the list
 * with the ActiveIndex of the stack.
 */
class AppRootView : public QWidget
{
    Q_OBJECT
   public:
    explicit AppRootView(QWidget* parent = nullptr);
    ~AppRootView() override = default;

    /**
     * @brief Injects the model and sets up synchronization logic.
     * @param model Pointer to the data model.
     */
    void setModel(AppRootModel* model);

    /**
     * @brief Sets the delegate for custom tab bar rendering.
     * @param delegate Pointer to the tab delegate.
     */
    void setDelegate(AppRootDelegate* delegate) const;

   private slots:
    /**
     * @brief Reacts to model changes to add tabs in the UI.
     */
    void onRowsInserted(const QModelIndex& parent, int first, int last) const;

    /**
     * @brief Reacts to model changes to remove tabs in the UI.
     */
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last) const;

    /**
     * @brief Handles metadata changes (title/icon) from the model.
     */
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                       const QList<int>& roles = QList<int>()) const;

    /**
     * @brief Handles tab switching to keep the stack in sync.
     */
    void handleTabChanged(int index) const;

   private:
    /**
     * @brief The tab bar populated by the options provided in the m_tabs.
     */
    QListView* m_tabView;

    /**
     * @brief The different Widgets of the tabs with one selected.
     */
    QStackedWidget* m_contentStack;

    /**
     * @brief Layout to order The m_tabBar and m_contentStack.
     */
    QVBoxLayout* m_mainLayout;

    /**
     * @brief Pointer to the injected model.
     */
    AppRootModel* m_model = nullptr;
};

}  // namespace AppRoot