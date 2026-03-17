/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
