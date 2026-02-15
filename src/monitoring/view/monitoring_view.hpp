#pragma once

#include <QLabel>
#include <QSortFilterProxyModel>
#include <QSplitter>

#include "can_bus_config_card.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "graph_list_view.hpp"
#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "signal_list.hpp"

/**
 * @namespace Monitoring
 * @brief Contains delegate and UI components for CAN signal monitoring.
 */
namespace Monitoring {
class MonitoringView : public QWidget
{
    Q_OBJECT
   public:
    explicit MonitoringView(MonitoringModel* model, MonitoringDelegate* delegate,
                            QWidget* parent = nullptr);
    ~MonitoringView() override = default;

    // Accessors for the Delegate to wire up signals/slots
    [[nodiscard]] auto getSignalListView() const -> SignalList*
    {
        return m_signalListView;
    }

    [[nodiscard]] auto getGraphListView() const -> GraphListView*
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
    void onUpdateMessages() const;

    void showNoDbcOverlay() const;
    void hideNoDbcOverlay() const;

   protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle() const;

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

    QWidget* m_noDbcOverlay;
    Core::TintedIconLabel* m_settingsIconLabel;
};
}  // namespace Monitoring
