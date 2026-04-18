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

#include <QList>
#include <QStackedWidget>
#include <QWidget>

#include "core/dto/replay_dto.hpp"
#include "core/widgets/sidebar.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "dbc_based_sending_subview.hpp"
#include "raw_sending_subview.hpp"
#include "replay_sending_subview.hpp"
#include "sending/model/sending_model.hpp"

namespace Sending {
class ReplaySendingSubView;

/**
 * @brief The main container view for the Sending tab.
 * Manages the sidebar navigation and the configuration header.
 */
class SendingView final : public QWidget
{
    Q_OBJECT

   public:
    explicit SendingView(QWidget* parent = nullptr);
    ~SendingView() override = default;

    // Accessors for the Delegate to wire up signals/slots
    [[nodiscard]] auto rawSubView() const -> RawSendingSubView*
    {
        return m_rawView;
    }
    [[nodiscard]] auto dbcSubView() const -> DbcSendingSubView*
    {
        return m_dbcView;
    }
    [[nodiscard]] auto replaySubView() const -> ReplaySendingSubView*
    {
        return m_replayView;
    }

    /**
     * @brief Updates the log session list shown in the replay subview.
     */
    void setLogSessions(const QList<Core::ReplaySessionInfo>& sessions);

    /** @brief Applies a predefined replay load-state in replay subview section 2. */
    void setReplayLoadState(ReplaySendingSubView::LoadState state);

    /** @brief Applies replay control-state for start/pause/resume/stop buttons. */
    void setReplayPlaybackState(ReplaySendingSubView::PlaybackState state);

    /** @brief Updates replay progress footer and progress bar. */
    void setReplayProgress(int currentFrame, int totalFrames);

    /** @brief Updates warning/error text in replay subview section 2. */
    void setReplayLoadWarningText(const QString& text);

    /** @brief Clears warning/error text in replay subview section 2. */
    void clearReplayLoadWarningText();

    /**
     * @brief Binds the View to the Model.
     * Use this to setup QDataWidgetMappers for the Raw view or
     * set the model on the DBC QListView.
     */
    void setModel(SendingModel* model);

   signals:
    /** @brief Emitted when the sidebar selection changes (0=Raw, 1=DBC) */
    void modeChanged(bool isDbcMode);

    /** @brief Emitted when user requests to start repeated sending */
    void startRepeatedSendingRequested(int intervalUs);

    /** @brief Emitted when user requests to stop repeated sending */
    void stopRepeatedSendingRequested();

    /** @brief Emitted when user requests a single send */
    void sendOnceRequested();

   public slots:
    /** @brief Switches the visible sub-view (0 for Raw, 1 for DBC) */
    auto displayMode(int index) -> void;

    /** @brief Shows the device not configured overlay */
    void showDeviceNotConfiguredOverlay() const;

    /** @brief Hides the device not configured overlay */
    void hideDeviceNotConfiguredOverlay() const;

   protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle() const;

    /** @brief Updates send button enabled states based on current selections */
    void updateSendButtonStates() const;

    // Sidebar
    Core::Sidebar* m_sidebar;

    QStackedWidget* m_contentStack;
    RawSendingSubView* m_rawView;
    DbcSendingSubView* m_dbcView;
    ReplaySendingSubView* m_replayView;

    // Overlay for device not configured
    QWidget* m_deviceNotConfiguredOverlay;
    Core::TintedIconLabel* m_settingsIconLabel;

    // Model reference for button state checks
    SendingModel* m_model = nullptr;
};

}  // namespace Sending
