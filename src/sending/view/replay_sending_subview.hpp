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
#include <QString>
#include <QWidget>
#include <cstdint>
#include <memory>

#include "core/interface/i_can_reader.hpp"
#include "core/interface/i_manipulation_handler.hpp"
#include "core/service/serializer.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/link_button.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "manipulation/ui/view/manipulation_view.hpp"

namespace Math {
class VariableRegistry;
}

namespace Sending {

/**
 * @struct ReplayEntry
 * @brief A replayable session discovered from disk, local to the Sending module.
 */
struct ReplayEntry {
    QString displayName;
    QString filePath;
    uint64_t frameCount = 0;
    Core::CanFileType fileType = Core::CanFileType::Raw;
};

class ReplayControlButton;
class ReplayProgressBar;

/**
 * @class ReplaySendingSubView
 * @brief Replay-specific subview for the Sending tab.
 *
 * Discovers available MF4 sessions from disk (populated by SendingComponent),
 * lets the user load and control playback without any round-trip to the Logging module.
 */
class ReplaySendingSubView final : public QWidget
{
    Q_OBJECT

   public:
    enum class PlaybackState { Disabled, Ready, Running, Paused };

    explicit ReplaySendingSubView(QWidget* parent = nullptr);
    ~ReplaySendingSubView() override = default;

    /**
     * @brief Replaces the list of available replay sessions shown in the combo box.
     */
    void setSessions(const QList<ReplayEntry>& entries);

    /** @brief Updates enabled state and visibility of the replay controls. */
    void setPlaybackState(PlaybackState state);

    /** @brief Returns a manipulation handler snapshot if injection is enabled, nullptr otherwise.
     */
    [[nodiscard]] auto getManipulationHandler() const
        -> std::shared_ptr<Core::IManipulationHandler>;

    /** @brief Updates the progress bar and frame counter label. */
    void setProgress(int currentFrame, int totalFrames);

    /** @brief Sets the DBC-aware registry used to populate signal/message dropdowns. */
    void setVariableRegistry(Math::VariableRegistry* registry);

   signals:
    /**
     * @brief Emitted when the user starts replay.
     * @param index Index into the list supplied by the last setSessions() call.
     * @param speed Playback speed factor.
     */
    void startReplayRequested(int index, double speed);

    void pauseReplayRequested();
    void resumeReplayRequested();
    void stopReplayRequested();

    /**
     * @brief Emitted when the user picks an external file via the Browse button.
     * @param filePath Absolute path to the selected file.
     */
    void externalFileSelected(const QString& filePath);

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi();
    void applyStyle() const;
    void updateSessionDetailsLabel();
    void updateManipulationMode();
    [[nodiscard]] auto selectedSpeedFactor() const -> double;

    /**
     * @brief Builds a serializer covering the full replay configuration: playback speed and
     * manipulations. Used identically for both save and load.
     */
    [[nodiscard]] auto buildStateSerializer() -> Core::Serializer;
    void onSaveClicked();
    void onLoadClicked();

    QScrollArea* m_scrollArea;

    Core::CardWidget* m_sessionCard;
    Core::StyledComboBox* m_sessionCombo;
    QLabel* m_sessionDetailsLabel;
    ReplayControlButton* m_browseButton;

    Manipulation::ManipulationView* m_manipulation;

    Core::CardWidget* m_playbackCard;
    ReplayControlButton* m_startButton;
    ReplayControlButton* m_pauseButton;
    ReplayControlButton* m_resumeButton;
    ReplayControlButton* m_stopButton;
    Core::StyledComboBox* m_speedCombo;

    Core::CardWidget* m_progressCard;
    ReplayProgressBar* m_progressBar;
    QLabel* m_progressTextLabel;

    // Configuration save/load
    Core::LinkButton* m_loadButton;
    Core::LinkButton* m_saveButton;

    QList<ReplayEntry> m_entries;
    PlaybackState m_playbackState = PlaybackState::Disabled;

    /** @brief Stored from the last setVariableRegistry() call, needed to check whether a DBC
     * config is loaded when a saved state contains DBC manipulations. */
    Math::VariableRegistry* m_registry = nullptr;
};

}  // namespace Sending