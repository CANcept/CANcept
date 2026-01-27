#pragma once

#include <QComboBox>
#include <QListView>
#include <QStackedWidget>
#include <QWidget>

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

    // UI Interaction API
    void setAvailableDevices(const std::vector<std::string>& devices);
    void setAvailableSpeeds(const std::vector<uint32_t>& speeds);

   signals:
    /** @brief Emitted when the sidebar selection changes (0=Raw, 1=DBC) */
    void modeChanged(bool isDbcMode);

    /**
     * @brief Emitted when the "Send Message" footer button is clicked.
     */
    void sendClicked();

    /** @brief Emitted when the device dropdown changes */
    void deviceSelectionChanged(const std::string& deviceName);

   public slots:
    /** @brief Switches the visible sub-view (0 for Raw, 1 for DBC) */
    void displayMode(int index);

   private slots:
    /**
     * @brief Handles sidebar navigation to switch between Raw and DBC views.
     */
    void onSidebarSelectionChanged(const QModelIndex& index);

   private:
    /**
     * @brief Describes a single entry in the sidebar model.
     */
    struct SidebarEntry {
        QString iconPath;
        QString title;
        bool enabled;
    };

    /** @brief Prevents deselection of items in the sidebar list. */
    void disableSidebarDeselection();

    /** @brief Initializes and configures the sidebar list view. */
    void setupSidebarList();

    /** @brief Sets up the model for the sidebar list. */
    void setSidebarModel();

    void setupUi();

    // Sidebar
    QListView* m_sidebarList;

    QStackedWidget* m_contentStack;
    RawSendingSubView* m_rawView;
    DbcSendingSubView* m_dbcView;
};

}  // namespace Sending