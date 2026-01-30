#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTreeView>

#include "can_bus_config_card.hpp"
#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"

/**
 * @namespace Monitoring
 * @brief Contains delegate and UI components for CAN signal monitoring.
 */
namespace Monitoring {
class GraphListView;
class MonitoringDelegate;
class MonitoringView : public QWidget
{
    Q_OBJECT
   public:
    explicit MonitoringView(MonitoringModel* model, MonitoringDelegate* delegate);
    ~MonitoringView() override = default;

    /**
     * Sets a model for the view.
     * @param model contains the model to be added
     */
    void setModel(MonitoringModel* model);

    /**
     * Sets a model for the view.
     * @param model contains the model to be added
     */
    void setDelegate(MonitoringDelegate* model);

    // Accessors for the Delegate to wire up signals/slots
    QTreeView* getTreeView() const
    {
        return m_signalsTreeView;
    }
    GraphListView* getGraphListView() const
    {
        return m_graphListView;
    }
   signals:

    /**
     * @brief Triggered when a signal is checked for plotting
     *
     * @param messageId the id of the message the checked signal belongs to
     * @param signalName the name of the checked signal
     */
    void signalChecked(char messageId, const std::string& signalName);

    /**
     * @brief Triggered when a signal is unchecked from plotting
     * Notifies GraphListView to clear graph list.
     *
     * @param messageId the id of the message the unchecked signal belongs to
     * @param signalName the name of the unchecked signal
     */
    void signalUnchecked(char messageId, const std::string& signalName);

   public slots:

    /**
     * @brief Triggered when the dbc configuration is changed
     * Notifies GraphListView to clear graph list.
     */
    void onDbcConfigurationChanged();

   private:
    void setupUi();

    /**
     * @brief Proxy model used to filter and sort the signal tree data.
     *
     * Enables search, category filtering, and dynamic visibility of signals
     * without modifying the underlying model.
     */
    QSortFilterProxyModel* m_treeProxy;  // For data filtering

    QTreeView* m_signalsTreeView;
    QSplitter* m_splitter;  // For Signals and Graphs scalable split view
    GraphListView* m_graphListView;

    MonitoringModel* m_model;
    MonitoringDelegate* m_delegate;

    CanBusConfigCard* m_configCard;
};
}  // namespace Monitoring
