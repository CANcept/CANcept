#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>

#include "can_bus_config_card.hpp"
#include "core/dto/dbc_dto.hpp"
#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "signal_list.hpp"

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
    [[nodiscard]] auto getSignalListView() const -> SignalList*
    {
        return m_signalListView;
    }

    [[nodiscard]] auto getGraphListView() const -> GraphListView*
    {
        return m_graphListView;
    }

    void stopTimer();

   signals:

    /**
     * @brief Triggered when a signal is checked for plotting
     *
     * @param messageId the id of the message the checked signal belongs to
     * @param signalName the name of the checked signal
     */
    void signalChecked(const QString* messageId, const QString& signalName);

    /**
     * @brief Triggered when a signal is unchecked from plotting
     * Notifies GraphListView to clear graph list.
     *
     * @param messageId the id of the message the unchecked signal belongs to
     * @param signalName the name of the unchecked signal
     */
    void signalUnchecked(const QString* messageId, const QString& signalName);

    void refreshDataView();

   public slots:
    void onUpdateMessages();

   private:
    void setupUi();

    /**
     * @brief Proxy model used to filter and sort the signal tree data.
     *
     * Enables search, category filtering, and dynamic visibility of signals
     * without modifying the underlying model.
     */
    QSortFilterProxyModel* m_treeProxy;

    SignalList* m_signalListView;
    QSplitter* m_splitter;
    GraphListView* m_graphListView;

    MonitoringModel* m_model;
    MonitoringDelegate* m_delegate;

    Core::DbcConfig m_dbcConfig;
};
}  // namespace Monitoring
