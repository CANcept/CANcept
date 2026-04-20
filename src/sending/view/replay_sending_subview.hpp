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
 * @brief Replay-specific subview for the Sending tab.
 *
 * The widget is responsible for selecting completed log sessions from the
 * Logging module, loading their frames, and controlling replay playback.
 * It also shows loading state messages, warnings, and replay progress.
 */
class ReplaySendingSubView final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @enum LoadState
     * @brief Describes the current frame-loading state of the replay workflow.
     */
    enum class LoadState {
        /** A session is selected and frames can be loaded. */
        SessionReady,
        /** A frame load request is currently in progress. */
        Loading,
        /** Frames have been loaded successfully. */
        Loaded,
        /** No completed log sessions are available. */
        NoSessions,
        /** Loading failed and an error message should be displayed. */
        Error
    };

    /**
     * @enum PlaybackState
     * @brief Describes the current playback state of the replay controls.
     */
    enum class PlaybackState {
        /** Replay controls are inactive because no frames are loaded. */
        Disabled,
        /** Frames are loaded and replay can be started. */
        Ready,
        /** Replay is currently running. */
        Running,
        /** Replay is currently paused. */
        Paused
    };

    /**
     * @brief Creates the replay subview.
     * @param parent Optional parent widget.
     */
    explicit ReplaySendingSubView(QWidget* parent = nullptr);
    ~ReplaySendingSubView() override = default;

    /**
     * @brief Replaces the available replay sessions shown in the combo box.
     * @param sessions Completed sessions received from the Logging module.
     */
    void setSessions(const QList<Core::ReplaySessionInfo>& sessions);

    /**
     * @brief Updates the loading state label in the replay status card.
     * @param state New loading state to display.
     */
    void setLoadState(LoadState state);

    /**
     * @brief Sets or shows a warning message in the loading status card.
     * @param text Warning text to display. An empty string hides the warning label.
     */
    void setWarningText(const QString& text);

    /**
     * @brief Clears the warning text in the loading status card.
     */
    void clearWarningText();

    /**
     * @brief Updates the enabled state and visibility of the replay controls.
     * @param state Current playback state.
     */
    void setPlaybackState(PlaybackState state);

    /**
     * @brief Updates the progress bar and textual frame counter.
     * @param currentFrame Number of frames that have already been processed.
     * @param totalFrames Total number of frames in the loaded replay session.
     */
    void setProgress(int currentFrame, int totalFrames);

   signals:
    /**
     * @brief Emitted when the user requests to load the selected session frames.
     * @param sessionId Identifier of the selected replay session.
     */
    void loadFramesRequested(const QString& sessionId);

    /**
     * @brief Emitted when the user requests replay playback.
     * @param speed Selected replay speed factor.
     */
    void startReplayRequested(double speed);

    /**
     * @brief Emitted when the user requests to pause replay.
     */
    void pauseReplayRequested();

    /**
     * @brief Emitted when the user requests to resume replay.
     */
    void resumeReplayRequested();

    /**
     * @brief Emitted when the user requests to stop replay.
     */
    void stopReplayRequested();

   protected:
    /**
     * @brief Handles style refresh events.
     * @param event Incoming Qt event.
     * @return True if the event was handled, otherwise false.
     */
    auto event(QEvent* event) -> bool override;

   private:
    /**
     * @brief Builds the subview layout and wires up local UI connections.
     */
    void setupUi();
    /**
     * @brief Applies the current theme to the subview widgets.
     */
    void applyStyle() const;
    /**
     * @brief Updates the session details label based on the current combo selection.
     */
    void updateSessionDetailsLabel();
    /**
     * @brief Returns the currently selected replay speed factor.
     * @return Selected speed factor as a multiplier.
     */
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
