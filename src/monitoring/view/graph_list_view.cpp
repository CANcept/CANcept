#include "graph_list_view.hpp"

#include <QScrollArea>
#include <QVBoxLayout>

namespace Monitoring {

GraphListView::GraphListView(MonitoringModel* model, MonitoringDelegate* delegate)
    : QWidget(nullptr), m_layout(new QVBoxLayout())
{
    // 1. Create the Scroll Area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    // 2. Create a container widget to hold the layout
    auto* container = new QWidget();
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(10);
    m_layout->setAlignment(Qt::AlignTop);  // Keep graphs at the top
    container->setLayout(m_layout);

    m_scrollArea->setWidget(container);

    // 3. Main layout for this widget
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
}

void GraphListView::addGraph(char messageId, const std::string& signalName)
{
    // Check if graph already exists to prevent duplicates
    for (auto* graph : m_signal_graphs)
    {
        if (graph->getSignalName() == signalName && graph->getMessageId() == messageId)
        {
            return;
        }
    }

    // Logic: In a real app, you'd fetch the Signal definition from your model
    // Assuming SignalGraph takes a name and ID for now
    SignalGraph* graph = new SignalGraph(messageId, signalName, this);
    m_signal_graphs.append(graph);
    m_layout->addWidget(graph);
}

void GraphListView::deleteGraph(char messageId, const std::string& signalName)
{
    for (int i = 0; i < m_signal_graphs.size(); ++i)
    {
        SignalGraph* graph = m_signal_graphs[i];
        if (graph->getSignalName() == signalName && graph->getMessageId() == messageId)
        {
            m_layout->removeWidget(graph);
            m_signal_graphs.removeAt(i);
            graph->deleteLater();  // Safe deletion in Qt
            return;
        }
    }
}

void GraphListView::appendDataToGraph()
{
    // This is typically called when new CAN data arrives.
    // You would iterate through your m_signal_graphs and update the
    // ones that match the incoming message ID.
}

void GraphListView::newGraph(Core::DbcCanSignal& signal)
{
    // Internal helper if you have a full Signal object available
    SignalGraph* graph = new SignalGraph(signal.value, signal.name, this);
    m_signal_graphs.append(graph);
    m_layout->addWidget(graph);
}

}  // namespace Monitoring