#pragma once


#include <QVBoxLayout>

#include "core/widgets/common/searchable_filter_widgets.hpp"

namespace DbcFile {

/**
 * @brief Page widget displaying the list of ECUs in a searchable and filterable tree view.
 *
 * The EcusPage provides a card-style header with title/subtitle,
 * a search bar, and a filter combo integrated with a tree view of ECUs.
 * It supports filtering by active/all ECUs and forwards filter/search events.
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
    void setModel(QAbstractItemModel* model) const;

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
     * @brief Sets up the UI elements.
     *
     * Orchestrates the creation of layout, header card,
     * tree section, tree view configuration, styling, and signal connections.
     */
    void setupUi();

private:
    // --- Modular UI setup helpers ---

    /**
     * @brief Creates the main layout for the page.
     *
     * Initializes the QVBoxLayout for the page and sets margins and spacing.
     */
    void createLayout();

    /**
     * @brief Creates the header card widget.
     *
     * Adds a Core::CardWidget with title and subtitle to the main layout.
     */
    void createHeaderCard();

    /**
     * @brief Creates the searchable and filterable tree section.
     *
     * Initializes m_treeWidget, sets placeholder text, and sets default filter options.
     */
    void createTreeSection();

    /**
     * @brief Configures the internal QTreeView.
     *
     * Applies selection behavior, row expansion, delegate assignment,
     * and hides the header/vertical headers as required.
     */
    void configureTreeView();

    /**
     * @brief Applies custom styling to the tree view branches and background.
     * @param view Pointer to the tree view to style.
     */
    static void applyTreeStyle(QTreeView* view);

    /**
     * @brief Connects signals from m_treeWidget to this page.
     */
    void connectSignals();

private:
    QLayout* m_cardLayout { nullptr };              ///< Layout of the card widget for tree placement
    Core::SearchableFilterTree* m_treeWidget { nullptr }; ///< Searchable and filterable tree widget
};

} // namespace DbcFile