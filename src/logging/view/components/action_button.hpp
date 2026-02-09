#pragma once

#include <QPushButton>

namespace Logging {

/**
 * @class ActionButton
 * @brief Styled toggle button for starting/stopping logging sessions.
 *
 * This component provides a consistent styled button that switches between
 * "Start" and "Stop" states with proper theming, icons, and hover effects.
 */
class ActionButton final : public QPushButton
{
    Q_OBJECT

   public:
    explicit ActionButton(QWidget* parent = nullptr);
    ~ActionButton() override = default;

    /**
     * @brief Sets the button to recording or idle state.
     * @param isRecording If true, shows "Stop" state; otherwise shows "Start" state
     */
    void setRecordingState(bool isRecording);

   private:
    void applyStyle();

    bool m_isRecording{false};
};

}  // namespace Logging
