#pragma once
#include <QVBoxLayout>

#include "core/widgets/tinted_icon_label.hpp"

namespace DbcFile {
/**
 * @class LoadPage
 * @brief The landing page for the DBC File module (SRS 6.1 "Load New").
 *
 * @details
 * VISUALS:
 * Renders a centered "Card" layout with an upload zone within, utilizing the
 * application's ThemeManager for colors and spacing.
 *
 * LOGIC:
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
     * @param parent Optional parent widget.
     */
    explicit LoadPage(QWidget* parent = nullptr);

    /**
     * @brief Destructor for LoadPage.
     */
    ~LoadPage() override;

    /**
     * @return @brief Getter for status text.
     */
QString testStatusText() const {
  return m_statusLabel ? m_statusLabel->text() : QString{};
 }

 /**
*
* @return @brief Getter for status visibility.
*/
 bool testStatusVisible() const {
  return m_statusLabel ? m_statusLabel->isVisible() : false;
 }
    /**
     * @brief Displays a status message in the upload zone.
     * @param message The text of the message.
     * @param isError True if this is an error message, false for success/info.
     *
     * @details Updates the label text, color, and visibility according to the theme.
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
     * - Accepts the proposed action so the drag operation may continue and be finalized by
     * `dropEvent`.
     */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @details
     * - Resets the visual border style.
     * - Performs final validation (count and extension).
     * - If invalid: Displays an inline error status message.
     * - If valid: Emits `fileSelected` and shows a parsing status message.
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

    /**
     * @brief Handles QEvent to catch StyleEvent for theme changes.
     */
    auto event(QEvent* event) -> bool override;

   private slots:
    /**
     * @brief Opens the system file dialog.
     * @details If a file is selected, emits `fileSelected` and updates status to "Parsing...".
     * If an invalid file is selected, shows QMessageBox::warning.
     */
    void onBrowseButtonClicked();

    /**
     * @brief Creates and styles the load card frame container.
     * @param parentLayout The main layout where the card should be added.
     * @param parent
     * @return The internal layout of the created card, used to add further content.
     */
    static auto createCardLayout(QVBoxLayout* parentLayout,
                                 QWidget* parent = nullptr) -> QVBoxLayout*;

    /**
     * @brief Creates the interactive Upload Zone frame.
     * @details Instantiates m_uploadBoxFrame, applies the drag-state stylesheet,
     * installs the event filter, and populates it with the icon and instruction text.
     * Adds the m_statusLabel (initially hidden).
     * @param layout The layout of the card frame.
     */
    void setupUploadZone(QVBoxLayout* layout);

   private:
    /**
     * @brief Orchestrates the complete UI initialization at construction.
     */
    void setupUi();

    /**
     * @brief Applies current theme styles to all widgets.
     */
    void applyStyle() const;

    /** @brief The interactive frame for dropping files and opening file browser. */
    QFrame* m_uploadBoxFrame;
    /** @brief Hidden label used to show parsing status or error messages. */
    QLabel* m_statusLabel;
    /** @brief Instruction label in upload zone */
    QLabel* m_instructionLabel;
    /** @brief Icon label in upload zone */
    Core::TintedIconLabel* m_iconLabel;
};
}  // namespace DbcFile
