#include "signal_graph.hpp"

#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>

#include <QVBoxLayout>
#include <utility>

namespace Monitoring {

SignalGraph::SignalGraph(char messageId, std::string signalName, QWidget* parent)
    : QWidget(parent),
      m_plot(new QwtPlot(this)),
      m_curve(new QwtPlotCurve()),
      m_messageId(messageId),
      m_signalName(std::move(signalName))
{
    setupPlot();
}

void SignalGraph::setupPlot()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Title and Style
    m_plot->setTitle(QString::fromStdString(m_signalName));
    m_plot->setCanvasBackground(Qt::white);

    // Add a grid
    auto* grid = new QwtPlotGrid();
    grid->attach(m_plot);

    // Configure the Curve
    m_curve->setTitle("Signal Value");
    m_curve->setPen(Qt::blue, 2);
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    // Attach curve to plot
    m_curve->attach(m_plot);

    layout->addWidget(m_plot);
    setLayout(layout);

    // Set a minimum height so the graphs don't collapse in the scroll area
    setMinimumHeight(250);
}

void SignalGraph::appendDataToGraph(Core::DbcCanSignal& signal)
{
    // 1. Update the internal model with the new raw sample
    // Your SignalGraphModel should handle the timestamping (X-axis)
    m_model.addValue(signal.value);

    // 2. Feed the updated data from the model to the Qwt curve
    // Qwt uses 'setSamples' to update the visual line
    m_curve->setSamples(m_model.timestamps(), m_model.values());

    // 3. Refresh the plot
    m_plot->replot();
}

SignalGraph::~SignalGraph()
{
    m_curve->detach();
    delete m_curve;
}

}  // namespace Monitoring
