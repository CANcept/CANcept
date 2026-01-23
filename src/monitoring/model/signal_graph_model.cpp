#include "signal_graph_model.hpp"

namespace Monitoring {

SignalGraphModel::SignalGraphModel(QObject* parent) : QObject(parent)
{
    m_timer.start();
}

void SignalGraphModel::addValue(double value)
{
    double currentTime = m_timer.elapsed() / 1000.0;

    m_timestamps.append(currentTime);
    m_values.append(value);

    // FIFO Logic: Remove data that has fallen off the left side of the window
    // We check if the (Newest Time - Oldest Time) > historyLimit
    while (!m_timestamps.isEmpty() && (currentTime - m_timestamps.first() > m_historyLimit))
    {
        m_timestamps.removeFirst();
        m_values.removeFirst();
    }
}

}  // namespace Monitoring