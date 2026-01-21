//
// Created by Adrian Rupp on 21.01.26.
//
#pragma once
#include <QLabel>
#include <QWidget>
namespace DbcFile {
/**
 * @class LoadPage
 * @brief The landing page for the DBC module (SRS 6.1 "Load New").
 *
 * @details
 * **VISUALS:**
 * Provides a centered "Card" layout.
 *
 * **LOGIC:**
 * Allows uploading a DBC file via Drag & Drop or by clicking to open a file dialog.
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

    void showStatusMessage(const QString& message, bool isError = false) const;
    void resetStatus() const;

   signals:
    /**
     * @brief Emitted when a file is successfully selected by the user.
     *
     * @details Triggered by either dropping a valid file onto the widget or selecting
     * one via the system file dialog.
     *
     * @param filePath The absolute path to the selected file.
     */
    void fileSelected(const QString& filePath);

   protected:
    /**
     * @brief Handles drag enter events to validate dropped data.
     * @caller Qt Event Loop (when dragging over widget).
     * @details Updates the UI (e.g., green border) if the dragged item is a dbc file.
     */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /**
     * @brief Handles drop events to extract the file path.
     * @caller Qt Event Loop (when mouse released).
     */
    void dropEvent(QDropEvent* event) override;

    /**
     * @brief Handles the event of leaving the upload area/zone while dragging data.
     * When drag leaves the upload area, the style of the upload area changes back to default.
     */
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    /**
     * @brief Event Filter for the upload Box.
     * @caller Qt Event Loop (before event reaches target).
     * @details Used to detect clicks on the central "Upload Box" frame to trigger the file dialog.
     */
    auto eventFilter(QObject* watched, QEvent* event) -> bool override;

   private slots:
    /**
     * @brief Opens a QFileDialog to let the user browse for a DBC file.
     * @caller Internal (on click event).
     */
    void onBrowseButtonClicked();

   private:
    /**
     * @brief Initializes the visual components and layout.
     * @caller Constructor.
     * @details Creates the central card frame, the upload box, and applies the stylesheets.
     */
    void setupUi();

    /** @brief The clickable area for file upload. */
    QFrame* m_uploadBoxFrame;
    /**
     * @brief A parsing status label within the area
     */
    QLabel* m_statusLabel;
};
}  // namespace DbcFile
