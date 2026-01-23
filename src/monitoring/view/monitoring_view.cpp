#include "monitoring_view.hpp"

#include <QHeaderView>
#include <QVBoxLayout>

#include "monitoring/delegate/monitoring_delegate.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

MonitoringView::MonitoringView()
    : QWidget(nullptr),
      m_treeProxy(new QSortFilterProxyModel(this)),
      m_signalsTreeView(new QTreeView(this)),
      m_splitter(new QSplitter(Qt::Horizontal, this)),
      m_graphListView(new GraphListView(m_model, m_delegate))
{
    setupUi();
}

void MonitoringView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Configure Tree View
    m_signalsTreeView->setAlternatingRowColors(true);
    m_signalsTreeView->header()->setStretchLastSection(true);
    m_signalsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_signalsTreeView->setSelectionMode(QAbstractItemView::SingleSelection);

    // Configure Splitter
    m_splitter->addWidget(m_signalsTreeView);
    m_splitter->addWidget(m_graphListView);

    // Give the Graph view more initial space (e.g., 30/70 split)
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(m_splitter);

    // Connect the data changed signal of the model to intercept checkbox toggles
    // We do this via the proxy so we get the correct indices
    connect(m_treeProxy, &QSortFilterProxyModel::dataChanged, this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                   const QVector<int>& roles) -> void {
                if (roles.contains(Qt::CheckStateRole) || roles.isEmpty())
                {
                    // In a tree, we check if the item now has a checkmark
                    bool isChecked = topLeft.data(Qt::CheckStateRole) == Qt::Checked;

                    // Extract data needed for the signals
                    // Note: We assume the model stores ID and Name in specific user roles or
                    // columns
                    char msgId = static_cast<char>(topLeft.data(Qt::UserRole + 1).toInt());
                    std::string sigName = topLeft.data(Qt::DisplayRole).toString().toStdString();

                    if (isChecked)
                    {
                        emit signalChecked(msgId, sigName);
                    } else
                    {
                        emit signalUnchecked(msgId, sigName);
                    }
                }
            });
}

void MonitoringView::setModel(QAbstractItemModel* model)
{
    if (model)
    {
        m_treeProxy->setSourceModel(model);
        m_signalsTreeView->setModel(m_treeProxy);

        // Expand the tree by default to show signals
        m_signalsTreeView->expandAll();
    }
}

void MonitoringView::setDelegate(QAbstractItemDelegate* delegate)
{
    if (delegate)
    {
        m_signalsTreeView->setItemDelegate(delegate);
    }
}

void MonitoringView::onDbcConfigurationChanged()
{
    // If the DBC changes, the old graphs are likely invalid
    if (m_graphListView)
    {
        // Assuming GraphListView has a clear or reset method
        // If not, we trigger the logic here
    }

    // Refresh the tree view
    m_signalsTreeView->expandAll();
}

}  // namespace Monitoring
