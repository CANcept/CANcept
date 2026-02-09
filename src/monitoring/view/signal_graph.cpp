#include "signal_graph.hpp"

#include <qwt_date_scale_draw.h>
#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>

#include <QVBoxLayout>
#include <utility>

#include "core/macro/console_logging.hpp"

namespace Monitoring {
SignalGraph::SignalGraph(QString messageId, QString signalName, QWidget* parent)
    : QWidget(parent),
      m_plot(new QwtPlot(this)),
      m_curve(new QwtPlotCurve()),
      m_grid(new QwtPlotGrid()),
      m_messageId(messageId),
      m_signalName(std::move(signalName))
{
    setupPlot();
}

void SignalGraph::setupPlot()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_plot->setCanvasBackground(Qt::white);

    m_grid->setPen(Qt::gray, 0, Qt::DashLine);
    m_grid->enableX(false);
    m_grid->enableY(false);
    m_grid->attach(m_plot);

    m_curve->setPen(Qt::black, 2);
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QwtDateScaleDraw* timeDraw = new QwtDateScaleDraw(Qt::LocalTime);

    timeDraw->setDateFormat(QwtDate::Hour, "HH:mm:ss");
    timeDraw->setDateFormat(QwtDate::Minute, "HH:mm:ss");
    timeDraw->setDateFormat(QwtDate::Second, "HH:mm:ss");
    timeDraw->setDateFormat(QwtDate::Millisecond, "HH:mm:ss");

    m_plot->setAxisScaleDraw(QwtPlot::xBottom, timeDraw);

    m_plot->enableAxis(QwtPlot::xBottom, false);
    m_plot->enableAxis(QwtPlot::yLeft, false);

    m_curve->attach(m_plot);

    QwtSymbol* symbol =
        new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::black), QPen(Qt::black, 1), QSize(5, 5));
    m_curve->setSymbol(symbol);

    layout->addWidget(m_plot);
    setLayout(layout);

    setMinimumHeight(300);
}

void SignalGraph::updateGraphData(QVariant timestamps, QVariant signalValues)
{
    QVector<double> tData = timestamps.value<QList<qreal>>().toVector();
    QVector<double> vData = signalValues.value<QList<qreal>>().toVector();

    if (tData.isEmpty() || vData.isEmpty()) return;

    if (!m_plot->axisEnabled(QwtPlot::xBottom))
    {
        m_plot->enableAxis(QwtPlot::xBottom, true);
        m_plot->enableAxis(QwtPlot::yLeft, true);
    }

    m_curve->setSamples(tData, vData);

    m_plot->setAxisScale(QwtPlot::xBottom, tData.first(), tData.last());

    if (m_grid)
    {
        m_grid->enableY(true);
        m_grid->enableX(true);
    }

    m_plot->replot();
}

SignalGraph::~SignalGraph()
{
    m_curve->detach();
    delete m_curve;
}

}  // namespace Monitoring
