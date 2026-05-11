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

#include <QFrame>
#include <QModelIndex>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "components/log_history_table.hpp"
#include "components/message_selection_dialog.hpp"
#include "components/no_logs_label.hpp"
#include "components/start_stop_button.hpp"
#include "components/timer_label.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "logging/model/logging_model.hpp"

namespace Logging {

/**
 * @class LoggingView
 * @brief The primary interface for the Logging module, implementing a multi-state Dashboard.
 * It handles the display of the previous log sessions and the detail view of a session.
 */
class LoggingView final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the LoggingView with a framed dashboard and internal stack.
     */
    explicit LoggingView(QWidget* parent = nullptr);

    /**
     * @brief Binds the Model to the internal TreeView and installs the strict Delegate.
     * @param model Pointer to the LoggingModel.
     */
    void setModel(LoggingModel* model);

    /** @brief Provides access to the tree view for Model/Delegate binding. */
    auto getHistoryTable() const -> LogHistoryTable*
    {
        return m_historyTable;
    }

    /** * @brief Switches the content frame to show the Detail View.
     * @details This hides the history table and populates the detail container.
     * @param detailWidget The widget containing specific log data (e.g., charts or signal lists).
     */
    void showDetailView(QWidget* detailWidget);

    /** * @brief Switches the content frame back to the History Table.
     */
    void hideDetailView();

    /**
     * @brief Updates the global Action button and Status label.
     * @param isRecording If true, UI reflects an active session (Red 'Stop' button).
     */
    void setRecordingState(bool isRecording);

    /**
     * @brief Updates the timer display during active logging.
     * @param elapsedMs The number of milliseconds elapsed since logging started.
     */
    void updateTimer(qint64 elapsedMs);

   signals:
    /** @brief Emitted when user wants to start; triggers the Modal Selection Dialog. */
    void startRequested(LogSessionType logSessionType,
                        const std::map<uint32_t, QStringList>& selectedSignals);
    /** @brief Emitted to stop the active session and finalize the log. */
    void stopRequested();

    /** @brief Triggered by a 'Details' button within a specific table row. */
    void detailRequested(const QModelIndex& index);

    /** @brief Forwarded from the component when a new DBC config is available. */
    void dbcConfigChanged(const Core::DbcConfig& config);

   public:
    /** @brief Shows the device not configured overlay. */
    void showDeviceNotConfiguredOverlay() const;
    /** @brief Hides the device not configured overlay. */
    void hideDeviceNotConfiguredOverlay() const;

   public slots:
    void onDetailRequested(const QModelIndex& index);

   protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private:
    /** @brief Initializes the persistent header and the swappable content frame. */
    void setupUi();
    void applyStyle();

    QWidget* m_headerBox;
    TimerLabel* m_timerLabel;
    StartStopButton* m_btnAction;

    NoLogsLabel* m_emptyLabel;
    bool m_isRecording{false};

    QFrame* m_mainFrame;
    QStackedWidget* m_contentStack;

    QWidget* m_historyPage;
    LogHistoryTable* m_historyTable;

    QWidget* m_detailPage;
    QVBoxLayout* m_detailLayout;

    QWidget* m_deviceNotConfiguredOverlay;
    Core::TintedIconLabel* m_settingsIconLabel;

    LoggingModel* m_currentModel{nullptr};

    std::unique_ptr<MessageSelectionDialog> m_selectionDialog;
};

}  // namespace Logging
