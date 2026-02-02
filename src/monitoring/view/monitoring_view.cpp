#include "monitoring_view.hpp"

#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>

#include "can_bus_config_card.hpp"
#include "core/dto/dbc_dto.hpp"
#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "graph_list_view.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

MonitoringView::MonitoringView(MonitoringModel* model, MonitoringDelegate* delegate)
    : QWidget(nullptr),
      m_treeProxy(new QSortFilterProxyModel(this)),
      m_signalListView(new SignalList(this)),
      m_splitter(new QSplitter(Qt::Horizontal, this))
{
    m_model = model;
    m_delegate = delegate;
    setupUi();
}

void MonitoringView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    m_graphListView = new GraphListView(m_model, m_delegate);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    m_configCard = new CanBusConfigCard(this);

    // If Core::CardWidget already has a layout, reuse it instead of adding a new one.
    if (m_configCard->layout() == nullptr)
    {
        auto* cardLayout = new QVBoxLayout(m_configCard);
        cardLayout->setContentsMargins(0, 0, 0, 0);
        cardLayout->setSpacing(spacing.spacingMd);
        m_configCard->setLayout(cardLayout);
    }
    // else: CanBusConfigCard/Core::CardWidget already created/set its own layout.

    // Configure Splitter
    m_splitter->addWidget(m_signalListView);
    m_splitter->addWidget(m_graphListView);

    // Give the Graph view more initial space (e.g., 30/70 split)
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    // --- Add to Main Layout ---
    mainLayout->addWidget(m_configCard, 1);
    mainLayout->addWidget(m_splitter, 5);

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
    // Connect checkboxes in signalListview to graphlistview
    connect(m_signalListView, &SignalList::signalMonitoringToggled, m_graphListView,
            &GraphListView::signalChecked);
}

void MonitoringView::onUpdateMessages()
{
    m_signalListView->updateViewData();
}

}  // namespace Monitoring
