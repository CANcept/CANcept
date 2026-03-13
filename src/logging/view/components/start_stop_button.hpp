#pragma once

#include <QPushButton>
#include <QTimer>

namespace Logging {

/**
 * @class StartStopButton
 * @brief Styled toggle button for starting/stopping logging sessions.
 *
 * This component provides a consistent styled button that switches between
 * "Start" and "Stop" states with proper theming, icons, and hover effects.
 */
class StartStopButton final : public QPushButton
{
    Q_OBJECT

   public:
    explicit StartStopButton(QWidget* parent = nullptr);
    ~StartStopButton() override = default;

    /**
     * @brief Sets the button to recording or idle state.
     * @param isRecording If true, shows "Stop" state; otherwise shows "Start" state
     */
    void setRecordingState(bool isRecording);

   protected:
    bool event(QEvent* event) override;

   private:
    void applyStyle();

    bool m_isRecording{false};
};

}  // namespace Logging
