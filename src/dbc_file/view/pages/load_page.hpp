//
// Created by Adrian Rupp on 21.01.26.
//
#pragma once
#include <QLabel>
#include <QWidget>
namespace DbcFile {
/**
 * @class LoadPage
 * @brief The landing page for the DBC File module (SRS 6.1 "Load New").
 *
 * @details
 * **VISUALS:**
 * Renders a centered "Card" layout with an upload zone within, utilizing the
 * application's ThemeManager for colors and spacing.
 *
 * **LOGIC:**
 * - Handles file uploads via Drag & Drop or File Dialog.
 * - Validates file extensions (*.dbc) and count (single file).
 * - Provides immediate visual feedback (borders colors, status messages).
 */
class LoadPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the LoadPage widget.
     * @caller DbcView::createSubViews().
     * @param parent The parent widget.
     */
    explicit LoadPage(QWidget* parent = nullptr);
    ~LoadPage() override;

    /**
     * @brief Displays a status message (Info or Error) inside the upload zone.
     *
     * @details
     * - Sets the text of the internal status label.
     * - Changes text color (Red for error, Blue for info).
     * - Updates the frame's border style (Red for error) via dynamic properties.
     *
     * @param message The text to display (e.g. "Parsing...").
     * @param isError If true, applies error styling; otherwise applies info styling.
     */
    void showStatusMessage(const QString& message, bool isError = false) const;

    /**
     * @brief Resets the UI to the default idle state.
     *
     * @details
     * Hides the status label, clears its text, and resets the upload box border
     * to the default style. Called when a new drag interaction begins.
     */
    void resetStatus() const;

   signals:
    /**
     * @brief Emitted when a valid file is selected by the user.
     * @param filePath The absolute path to the selected file.
     */
    void fileSelected(const QString& filePath);

   protected:
    /**
    * @brief Validates dragged data when it enters the widget area.
    *
    * @details
    * - Resets previous status messages.
    * - Checks if the mime data contains exactly one file ending in `.dbc`.
    * - Updates the border style (Green for valid, Red for invalid).
    * - Always accepts the proposed action to allow `dropEvent` to handle final logic.
    */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @brief Handles the final drop action.
     *
     * @details
     * - Resets the visual border style.
     * - Performs final validation (count and extension).
     * - If invalid: Shows a QMessageBox warning.
     * - If valid: Emits `fileSelected` and calls `showStatusMessage("Parsing...")`.
     */
    void dropEvent(QDropEvent* event) override;

    /**
     * @brief Resets visual feedback when a drag operation leaves the widget.
     */
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    /**
     * @brief Intercepts events for child widgets.
     * @details Used to detect `MouseButtonRelease` on the `m_uploadBoxFrame`
     * to trigger the file browser dialog.
     */
    auto eventFilter(QObject* watched, QEvent* event) -> bool override;

   private slots:
    /**
     * @brief Opens the system file dialog.
     * @details If a file is selected, emits `fileSelected` and updates status to "Parsing...".
     * If an invalid file is selected, shows QMessageBox::warning.
     */
    void onBrowseButtonClicked();

   private:
    /**
         * @brief Initializes the visual components, layout structure, and styling.
         *
         * @details
         * 1. Retrieves theme colors/spacing from Core::ThemeManager.
         * 2. Creates the main centered card frame.
         * 3. Creates the Title and Subtitle labels.
         * 4. Constructs the interactive `m_uploadBoxFrame` and installs the event filter.
         * 5. Populates the upload box with the SVG Icon and instruction text.
         * 6. Initializes the hidden `m_statusLabel` for feedback messages.
         */
    void setupUi();

    /** @brief The interactive frame for dropping files and opening file browser. */
    QFrame* m_uploadBoxFrame;
    /** @brief Hidden label used to show parsing status or error messages. */
    QLabel* m_statusLabel;
};
}  // namespace DbcFile
