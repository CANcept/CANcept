#pragma once

#include <QLabel>
#include <cstdint>

namespace Logging {

/**
 * @class TimerLabel
 * @brief Styled label displaying elapsed time in HH:MM:SS:CS format.
 *
 * This component provides a consistent styled label for displaying
 * the elapsed time during recording sessions.
 */
class TimerLabel final : public QLabel
{
    Q_OBJECT

   public:
    explicit TimerLabel(QWidget* parent = nullptr);
    ~TimerLabel() override = default;

    /**
     * @brief Updates the timer display with elapsed time.
     * @param elapsedMs The number of milliseconds elapsed since logging started
     */
    void updateTimer(qint64 elapsedMs);

   private:
    void applyStyle();
};

}  // namespace Logging
