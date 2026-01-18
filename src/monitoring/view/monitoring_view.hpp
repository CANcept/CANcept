#pragma once

#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTreeView>

#include "graph_list_view.hpp"

/**
 * @namespace Monitoring
 * @brief Contains delegate and UI components for CAN signal monitoring.
 */
namespace Monitoring {
class MonitoringView : public QWidget
{
   public:
    explicit MonitoringView(QWidget* parent = nullptr);
    ~MonitoringView() override = default;
    // Accessors for the Delegate to wire up signals/slots
    QTreeView* getTreeView() const
    {
        return m_signalsTreeView;
    }
    GraphListView* getGraphListView() const
    {
        return m_graphListView;
    }

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
};
}  // namespace Monitoring
