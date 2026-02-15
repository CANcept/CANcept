#pragma once

#include <QAbstractItemModel>
#include <QLabel>
#include <QVBoxLayout>

#include "core/widgets/common/searchable_filter_widgets.hpp"

namespace DbcFile {

/**
 * @class EcusPage
 * @brief Page widget displaying a list of ECUs in a searchable and filterable tree view.
 *
 * The EcusPage consists of:
 * - A card-style header with title and subtitle.
 * - A search bar and filter combo integrated with a tree view of ECUs.
 * - A placeholder label shown when no ECUs are available.
 *
 * It supports filtering by active/all ECUs and emits signals when
 * the search text or filter index changes.
 */
class EcusPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs an EcusPage.
     * @param parent Optional parent widget.
     */
    explicit EcusPage(QWidget* parent = nullptr);

    /**
     * @brief Sets the model for the internal tree view.
     * @param model The item model to display in the tree.
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Updates the empty state label visibility depending on tree contents.
     */
    void updateEmptyState();

    /**
     * @brief Handles custom events for styling.
     *
     * Refreshes style for child widgets if a Core::StyleEvent is received.
     * @param event The event object.
     * @return True if the event was handled.
     */
    bool event(QEvent* event) override;

   signals:
    /**
     * @brief Emitted when the search text in the filter bar changes.
     * @param text Current search string.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter selection index changes.
     * @param index Current filter index.
     */
    void filterIndexChanged(int index);

   protected:
    /**
     * @brief Sets up the page UI.
     *
     * Orchestrates the creation of layout, header card,
     * tree section, tree view configuration, styling, and signal connections.
     */
    void setupUi();

   private:
    // --- UI Setup Helpers ---

    /**
     * @brief Creates the main vertical layout for the page.
     *
     * Initializes QVBoxLayout and sets default margins and spacing.
     */
    void createLayout();

    /**
     * @brief Creates the card header widget.
     *
     * Adds a Core::CardWidget with title and subtitle to the main layout.
     */
    void createHeaderCard();

    /**
     * @brief Creates the searchable/filterable tree section.
     *
     * Initializes m_treeWidget, sets placeholder text, default filter options,
     * and the empty-state label.
     */
    void createTreeSection();

    /**
     * @brief Configures the internal QTreeView.
     *
     * Assigns delegate, sets selection behavior, expands/collapses rows,
     * hides the header, and applies other view settings.
     */
    void configureTreeView();

    /**
     * @brief Updates styles for the tree view and scrollbars.
     *
     * This method applies the current theme to the tree, including
     * the vertical scrollbar, and should be called whenever the
     * theme is updated at runtime.
     */
    void applyStyle();

    /**
     * @brief Connects signals from m_treeWidget to this page.
     */
    void connectSignals();

   private:
    QLayout* m_cardLayout{nullptr};  ///< Layout of the card header for tree placement
    Core::SearchableFilterTree* m_treeWidget{nullptr};  ///< Searchable/filterable tree of ECUs
    QLabel* m_emptyLabel{nullptr};                      ///< Label shown when no ECUs are available
};

}  // namespace DbcFile