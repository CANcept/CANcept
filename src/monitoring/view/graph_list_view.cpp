#include "graph_list_view.hpp"

#include <QScrollArea>
#include <QVBoxLayout>

#include "core/macro/console_logging.hpp"
#include "core/widgets/card_widget.hpp"
#include "monitoring/constants.hpp"
#include "signal_graph.hpp"

namespace Monitoring {

GraphListView::GraphListView(MonitoringModel* model, MonitoringDelegate* delegate)
    : QWidget(nullptr), m_layout(new QVBoxLayout()), m_model(model)
{
    // Scroll Area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    // widget to hold the layout
    auto* container = new QWidget();
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(10);
    m_layout->setAlignment(Qt::AlignTop);  // Keep graphs at the top
    container->setLayout(m_layout);

    m_scrollArea->setWidget(container);

    // main layout for this widget
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
}

void GraphListView::addGraph(const QString& messageId, const QString& signalName)
{
    for (auto* graph : m_signal_graphs)
    {
        if (graph->getSignalName() == signalName && graph->getMessageId() == messageId)
        {
            return;
        }
    }

    LOG_INF("MonitoringComponent", "GraphList graph added for signal");

    SignalGraph* graph = new SignalGraph(messageId, signalName, nullptr);
    graph->setContainer(new Core::CardWidget(QString("0x%1:  %2").arg(messageId, signalName),
                                             QString(), Constants::SIGNAL_GRAPH_ICON_PATH, this));
    graph->getContainer()->contentLayout()->addWidget(graph);

    m_signal_graphs.append(graph);
    m_layout->addWidget(graph->getContainer());
}

void GraphListView::deleteGraph(const QString& messageId, const QString& signalName)
{
    for (int i = 0; i < m_signal_graphs.size(); ++i)
    {
        SignalGraph* graph = m_signal_graphs[i];
        if (graph->getSignalName() == signalName && graph->getMessageId() == messageId)
        {
            QWidget* container = graph->getContainer();

            if (container) container->hide();
            graph->hide();

            m_signal_graphs.removeAt(i);

            if (container)
            {
                m_layout->removeWidget(container);
                container->setParent(nullptr);
                container->deleteLater();
            } else
            {
                graph->setParent(nullptr);
                graph->deleteLater();
            }

            LOG_INF("MonitoringComponent", "Graph deletion complete.");
            return;
        }
    }
}

void GraphListView::signalChecked(bool checked, const QString& messageId, const QString& signalName)
{
    if (checked)
    {
        addGraph(messageId, signalName);
    } else
    {
        deleteGraph(messageId, signalName);
    }
}

void GraphListView::onDbcChange()
{
    for (SignalGraph* graph : m_signal_graphs)
    {
        m_layout->removeWidget(graph->getContainer());
        graph->deleteLater();
    }
    m_signal_graphs.clear();
}

void GraphListView::updateViewData()
{
    if (!m_model) return;

    for (auto* weakGraph : m_signal_graphs)
    {
        if (!weakGraph) continue;

        SignalGraph* graph = weakGraph;
        QString targetMsgId = graph->getMessageId();
        QString targetSignalName = graph->getSignalName();
        QModelIndex messageIndex;

        bool found = false;
        for (int i = 0; i < m_model->rowCount(QModelIndex()); ++i)
        {
            QModelIndex checkIndex = m_model->index(i, 0, QModelIndex());
            QString currentId =
                m_model->data(checkIndex, MonitoringModel::MonitoringRoles::Role_ID).toString();

            if (currentId == targetMsgId)
            {
                messageIndex = checkIndex;
                found = true;
                break;
            }
        }

        if (!found)
        {
            continue;
        }

        int signalCount = m_model->rowCount(messageIndex);

        for (int j = 0; j < signalCount; ++j)
        {
            QModelIndex signalIndex = m_model->index(j, 0, messageIndex);

            QString currentSignalName =
                m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();

            if (currentSignalName == targetSignalName)
            {
                QVariant timestamps =
                    m_model->data(messageIndex, MonitoringModel::MonitoringRoles::Role_ValueList);

                QVariant signalValues =
                    m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_ValueList);

                graph->updateGraphData(timestamps, signalValues);

                break;
            }
        }
    }
}

}  // namespace Monitoring