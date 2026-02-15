//
// Logging view header
//

#pragma once

#include <QFrame>
#include <QModelIndex>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "components/action_button.hpp"
#include "components/empty_state_label.hpp"
#include "components/history_table.hpp"
#include "components/status_tags_container.hpp"
#include "components/timer_label.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
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
    auto getHistoryTable() const -> HistoryTable*
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

    /**
     * @brief Updates the message status tags shown during recording.
     * @param messages List of message IDs/names to display as tags.
     */
    void updateStatusTags(const QStringList& messages);

    /**
     * @brief Returns whether DBC-based logging is enabled.
     * @return true if DBC-based logging is enabled, false for raw logging.
     */
    bool isDbcLoggingEnabled() const;

   signals:
    /** @brief Emitted when user wants to start; triggers the Modal Selection Dialog. */
    void startRequested();
    /** @brief Emitted to stop the active session and finalize the log. */
    void stopRequested();
    /** @brief Emitted when logging mode changes (DBC-based vs raw). */
    void loggingModeChanged(bool dbcBased);

    /** @brief Triggered by an 'Export' button within a specific table row. */
    void exportRequested(const QModelIndex& index);
    /** @brief Triggered by a 'Details' button within a specific table row. */
    void detailRequested(const QModelIndex& index);

   private:
    /** @brief Initializes the persistent header and the swappable content frame. */
    void setupUi();

    QWidget* m_headerBox;
    TimerLabel* m_timerLabel;               /**< Displays elapsed time during recording. */
    ActionButton* m_btnAction;              /**< The Start/Stop toggle button. */
    Core::StyledCheckBox* m_dbcCheckbox;    /**< Checkbox to toggle DBC-based logging. */
    StatusTagsContainer* m_statusContainer; /**< Container for message status tags. */

    EmptyStateLabel* m_emptyLabel; /**< Empty state placeholder when no sessions exist. */
    bool m_isRecording{false};

    QFrame* m_mainFrame;            /**< The bordered container for consistent UI. */
    QStackedWidget* m_contentStack; /**< Handles swapping between Table and Details. */

    QWidget* m_historyPage;
    HistoryTable* m_historyTable;

    QWidget* m_detailPage;
    QVBoxLayout* m_detailLayout;
};

}  // namespace Logging
