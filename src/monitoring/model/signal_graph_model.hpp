#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QVector>

#include "core/dto/can_dto.hpp"

/**
 * @namespace Monitoring
 * @brief Contains models and UI components for CAN signal visualization.
 */
namespace Monitoring {

/**
 * @class SignalGraphModel
 * @brief Model managing time-series data for a visualized CAN signal.
 *
 * SignalGraphModel represents the data layer for a single signal graph.
 * It stores incoming signal values, manages their lifecycle, and provides
 * the data required by the corresponding SignalGraph view.
 *
 * The model is intentionally UI-agnostic and communicates exclusively
 * through Qt signals and slots.
 */
class SignalGraphModel : public QObject
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs the signal graph model.
     *
     * @param parent Optional Qt parent object.
     */
    explicit SignalGraphModel(QObject* parent = nullptr);

    // Getters for Qwt
    const QVector<double>& timestamps() const
    {
        return m_timestamps;
    }

    const QVector<double>& values() const
    {
        return m_values;
    }

    void addValue(double value);

    // Set how many seconds of history to keep (e.g., 10.0)
    void setHistoryLimit(double seconds)
    {
        m_historyLimit = seconds;
    }

   private:
    QVector<double> m_timestamps;
    QVector<double> m_values;
    QElapsedTimer m_timer;

    double m_historyLimit = 10.0;  // Default to 10 seconds of rolling data
};
}  // namespace Monitoring
