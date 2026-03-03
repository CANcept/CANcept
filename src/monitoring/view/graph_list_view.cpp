#include "graph_list_view.hpp"

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/styles.hpp"
#include "signal_graph.hpp"

namespace Monitoring {

GraphListView::GraphListView(MonitoringModel* model, MonitoringDelegate* delegate)
    : QWidget(nullptr),
      m_layout(new QVBoxLayout()),
      m_scrollArea(new QScrollArea(this)),
      m_model(model)
{
    setupUi();
}

void GraphListView::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_scrollArea = new QScrollArea(this);
    m_scrollContent = new QWidget(m_scrollArea);
    m_layout = new QVBoxLayout(m_scrollContent);

    m_scrollArea->setWidget(m_scrollContent);
    m_mainLayout->addWidget(m_scrollArea);

    applyStyle();
}

void GraphListView::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    if (!m_scrollContent) return;
    m_scrollContent->setStyleSheet(QString("background: %1;").arg(colors.surfaceMain.name()));

    if (!m_mainLayout) return;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    if (!m_scrollArea) return;
    m_scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    if (!m_layout) return;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(spacing.spacingSm);
    m_layout->addStretch();

    // Apply vertical scrollbar style
    if (m_scrollArea->verticalScrollBar())
        m_scrollArea->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());
}

auto GraphListView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        updateViewData();

        for (SignalGraph* graph : m_signal_graphs)
        {
            graph->applyStyle();
        }

        return true;
    }
    return QWidget::event(event);
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

    auto* graph = new SignalGraph(messageId, signalName, this);
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
        if (SignalGraph* graph = m_signal_graphs[i];
            graph->getSignalName() == signalName && graph->getMessageId() == messageId)
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
    for (const SignalGraph* graph : m_signal_graphs)
    {
        deleteGraph(graph->getMessageId(), graph->getSignalName());
    }
    m_signal_graphs.clear();
}

void GraphListView::updateViewData()
{
    if (!m_model) return;

    for (SignalGraph* graph : m_signal_graphs)
    {
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

        const int signalCount = m_model->rowCount(messageIndex);

        for (int j = 0; j < signalCount; ++j)
        {
            QModelIndex signalIndex = m_model->index(j, 0, messageIndex);

            QString currentSignalName =
                m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();

            if (currentSignalName == targetSignalName)
            {
                const QVariant timestamps =
                    m_model->data(messageIndex, MonitoringModel::MonitoringRoles::Role_ValueList);

                const QVariant signalValues =
                    m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_ValueList);

                graph->updateGraphData(timestamps, signalValues);

                break;
            }
        }
    }
}

}  // namespace Monitoring
