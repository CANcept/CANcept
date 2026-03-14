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

    /** @brief Triggered by an 'Export' button within a specific table row. */
    void exportRequested(const QModelIndex& index);
    /** @brief Triggered by a 'Details' button within a specific table row. */
    void detailRequested(const QModelIndex& index);

   public:
    /** @brief Shows the device not configured overlay. */
    void showDeviceNotConfiguredOverlay() const;
    /** @brief Hides the device not configured overlay. */
    void hideDeviceNotConfiguredOverlay() const;

   public slots:
    void dbcConfigChanged(const Core::DbcConfig& config);
    void onDetailRequested(const QModelIndex& index);

   protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private:
    /** @brief Initializes the persistent header and the swappable content frame. */
    void setupUi();
    void applyStyle();
    /** @brief Builds a detail widget for a specific session */
    auto createDetailWidget(const LogSession* session) -> QWidget*;

    QWidget* m_headerBox;
    TimerLabel* m_timerLabel;     /**< Displays elapsed time during recording. */
    StartStopButton* m_btnAction; /**< The Start/Stop toggle button. */

    NoLogsLabel* m_emptyLabel; /**< Empty state placeholder when no sessions exist. */
    bool m_isRecording{false};

    QFrame* m_mainFrame;            /**< The bordered container for consistent UI. */
    QStackedWidget* m_contentStack; /**< Handles swapping between Table and Details. */

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
