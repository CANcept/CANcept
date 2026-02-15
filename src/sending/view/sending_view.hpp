#pragma once

#include <QStackedWidget>
#include <QWidget>

#include "core/widgets/sidebar.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "dbc_based_sending_subview.hpp"
#include "raw_sending_subview.hpp"
#include "sending/model/sending_model.hpp"

namespace Sending {

/**
 * @brief The main container view for the Sending tab.
 * Manages the sidebar navigation and the configuration header.
 */
class SendingView final : public QWidget
{
    Q_OBJECT

   public:
    explicit SendingView(QWidget* parent = nullptr);
    ~SendingView() override = default;

    // Accessors for the Delegate to wire up signals/slots
    RawSendingSubView* rawSubView() const
    {
        return m_rawView;
    }
    DbcSendingSubView* dbcSubView() const
    {
        return m_dbcView;
    }

    /**
     * @brief Binds the View to the Model.
     * Use this to setup QDataWidgetMappers for the Raw view or
     * set the model on the DBC QListView.
     */
    void setModel(SendingModel* model);

   signals:
    /** @brief Emitted when the sidebar selection changes (0=Raw, 1=DBC) */
    void modeChanged(bool isDbcMode);

    /** @brief Emitted when user requests to start repeated sending */
    void startRepeatedSendingRequested(int intervalMs);

    /** @brief Emitted when user requests to stop repeated sending */
    void stopRepeatedSendingRequested();

    /** @brief Emitted when user requests a single send */
    void sendOnceRequested();

   public slots:
    /** @brief Switches the visible sub-view (0 for Raw, 1 for DBC) */
    void displayMode(int index);

    /** @brief Shows the device not configured overlay */
    void showDeviceNotConfiguredOverlay() const;

    /** @brief Hides the device not configured overlay */
    void hideDeviceNotConfiguredOverlay() const;

   protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle() const;

    /** @brief Updates send button enabled states based on current selections */
    void updateSendButtonStates() const;

    // Sidebar
    Core::Sidebar* m_sidebar;

    QStackedWidget* m_contentStack;
    RawSendingSubView* m_rawView;
    DbcSendingSubView* m_dbcView;

    // Overlay for device not configured
    QWidget* m_deviceNotConfiguredOverlay;
    Core::TintedIconLabel* m_settingsIconLabel;

    // Model reference for button state checks
    SendingModel* m_model = nullptr;
};

}  // namespace Sending
