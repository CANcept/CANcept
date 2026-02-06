#pragma once

#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "app_root/delegate/app_root_delegate.hpp"
#include "app_root/model/app_root_model.hpp"
#include "app_root/model/settings_model.hpp"
#include "app_root/view/settings_view.hpp"
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

    void setModel(AppRootModel* model);
    void setDelegate(AppRootDelegate* delegate) const;
    void setSettingsModel(SettingsModel* settingsModel);

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
    void handleTabChanged(int index);
    /**
     * @brief Handles the switch to the settings part.
     */
    void onSettingsClicked();

   protected:
    /**
     * @brief Filters events for the tab view to prevent selection ghosting.
     * * Intercepts mouse events on the tab bar's viewport to ignore clicks on
     * empty space, ensuring the current selection remains active and focused.
     * * @param watched The object being monitored (m_tabView viewport).
     * @param event The triggered event.
     * @return true if the event was handled (swallowed), false otherwise.
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * Triggered when the Widget shows itself. Then the first element should be selected.
     * @param event the event given on show
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Handles window resize events to update tab bar sizing.
     * @param event The resize event containing old and new sizes.
     */
    void resizeEvent(QResizeEvent* event) override;

    /**
     * @brief Handes the rerender of the AppRootView.
     * @param event The received event.
     */
    bool event(QEvent* event) override;

   private:
    void updateSettingsButtonStyle(bool active);

    void applyStyle();

    QListView* m_tabView;
    QStackedWidget* m_contentStack;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_topBarLayout{nullptr};
    QLabel* m_logoLabel{nullptr};
    QPushButton* m_settingsButton{nullptr};
    SettingsView* m_settingsView{nullptr};
    AppRootModel* m_model{nullptr};
    SettingsModel* m_settingsModel{nullptr};
    bool m_settingsActive{false};
    int m_lastTabIndex{0};
};

}  // namespace AppRoot