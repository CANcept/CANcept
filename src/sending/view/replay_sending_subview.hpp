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

#include <QLabel>
#include <QScrollArea>
#include <QWidget>

#include "core/dto/replay_dto.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"

namespace Sending {
class ReplayControlButton;
class ReplayProgressBar;

/**
 * @class ReplaySendingSubView
 * @brief Skeleton view for replay workflow in the Sending tab.
 *
 * This first version intentionally implements only section 1 (session selection + metadata).
 */
class ReplaySendingSubView final : public QWidget
{
    Q_OBJECT

   public:
    enum class LoadState {
        SessionReady,
        Loading,
        Loaded,
        NoSessions,
        Error
    };

    enum class PlaybackState {
        Disabled,
        Ready,
        Running,
        Paused
    };

    explicit ReplaySendingSubView(QWidget* parent = nullptr);
    ~ReplaySendingSubView() override = default;

    /**
     * @brief Replaces available replay sessions and updates the first-section UI.
     */
    void setSessions(const QList<Core::ReplaySessionInfo>& sessions);

    /** @brief Applies a predefined load-state text for section 2. */
    void setLoadState(LoadState state);

    /** @brief Updates warning/error text in section 2. */
    void setWarningText(const QString& text);

    /** @brief Clears warning/error text in section 2. */
    void clearWarningText();

    /** @brief Applies enabled/disabled logic for replay controls. */
    void setPlaybackState(PlaybackState state);

    /** @brief Updates replay progress bar and footer text. */
    void setProgress(int currentFrame, int totalFrames);

   signals:
    /** @brief Emitted when user requests loading frames for the selected session. */
    void loadFramesRequested(const QString& sessionId);

    /** @brief Emitted when user requests replay start with selected speed factor. */
    void startReplayRequested(double speed);

    /** @brief Emitted when user requests replay pause. */
    void pauseReplayRequested();

    /** @brief Emitted when user requests replay resume. */
    void resumeReplayRequested();

    /** @brief Emitted when user requests replay stop. */
    void stopReplayRequested();

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi();
    void applyStyle() const;
    void updateSessionDetailsLabel();
    [[nodiscard]] auto selectedSpeedFactor() const -> double;

    QScrollArea* m_scrollArea;

    // Section 1: Session source / selection
    Core::CardWidget* m_sessionCard;
    Core::StyledComboBox* m_sessionCombo;
    QLabel* m_sessionDetailsLabel;
    ReplayControlButton* m_loadFramesButton;

    // Section 2: Frame loading state
    Core::CardWidget* m_loadStateCard;
    QLabel* m_loadStatusLabel;
    QLabel* m_warningLabel;

    // Section 3: Playback controls
    Core::CardWidget* m_playbackCard;
    ReplayControlButton* m_startButton;
    ReplayControlButton* m_pauseButton;
    ReplayControlButton* m_resumeButton;
    ReplayControlButton* m_stopButton;
    Core::StyledComboBox* m_speedCombo;

    // Section 4: Progress & status footer
    Core::CardWidget* m_progressCard;
    ReplayProgressBar* m_progressBar;
    QLabel* m_progressTextLabel;

    QList<Core::ReplaySessionInfo> m_sessions;
    PlaybackState m_playbackState = PlaybackState::Disabled;
};

}  // namespace Sending
