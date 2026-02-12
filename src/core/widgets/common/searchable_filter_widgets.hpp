/**
 * @file searchable_filter_widgets.hpp
 * @brief Composite widgets combining a filter bar with a table or tree view.
 *
 * Provides reusable UI components that integrate:
 * - A StyledFilterBar (search field + optional combo filter)
 * - A QTableView or QTreeView
 *
 * The widgets forward filter signals to allow higher-level pages
 * to connect proxy models or filtering logic.
 */

#pragma once

#include <QWidget>

class QTableView;
class QTreeView;

namespace Core {

class StyledFilterBar;

// ============================================================================
// SearchableFilterTable
// ============================================================================

/**
 * @class SearchableFilterTable
 * @brief Composite widget combining a StyledFilterBar and a QTableView.
 *
 * Structure:
 *  - Top: StyledFilterBar (search + optional filter combo)
 *  - Bottom: QTableView wrapped in a styled frame
 *
 * Responsibilities:
 *  - Forwards filter signals (search text, filter index)
 *  - Applies consistent theme-based styling to the table and header
 *  - Provides access to the internal QTableView for model assignment
 */
class SearchableFilterTable : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the composite table widget.
     * @param parent Optional parent widget.
     */
    explicit SearchableFilterTable(QWidget* parent = nullptr);

    /**
     * @brief Returns the internal QTableView.
     */
    [[nodiscard]] auto tableView() const -> QTableView*;

    /**
     * @brief Returns the internal StyledFilterBar.
     */
    [[nodiscard]] auto filterBar() const -> StyledFilterBar*;

    /**
     * @brief Sets the placeholder text of the search field.
     */
    void setSearchPlaceholder(const QString& text) const;

    /**
     * @brief Sets the filter combo box options.
     */
    void setFilterOptions(const QStringList& options) const;

    /**
     * @brief Sets the current search text.
     */
    void setSearchText(const QString& text) const;

    /**
     * @brief Applies themed styling to the horizontal header.
     */
    void configureHeaderStyle();

    /**
     * @brief Applies themed styling to the table view.
     */
    void applyTableStyle();

    /**
     * @brief Configures base table behavior (scrollbars, grid, headers).
     */
    void configureTableBasics();

signals:
    /**
     * @brief Emitted when the search text changes.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter combo index changes.
     */
    void filterIndexChanged(int index);

private:
    /**
     * @brief Builds the widget layout and connects filter signals.
     */
    void setupUi();

private:
    StyledFilterBar* m_filterBar = nullptr;
    QTableView* m_tableView = nullptr;
};


// ============================================================================
// SearchableFilterTree
// ============================================================================

/**
 * @class SearchableFilterTree
 * @brief Composite widget combining a StyledFilterBar and a QTreeView.
 *
 * Structure:
 *  - Top: StyledFilterBar (search + optional filter combo)
 *  - Bottom: QTreeView
 *
 * Responsibilities:
 *  - Forwards filter signals
 *  - Provides access to the internal QTreeView for model assignment
 */
class SearchableFilterTree : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the composite tree widget.
     * @param parent Optional parent widget.
     */
    explicit SearchableFilterTree(QWidget* parent = nullptr);

    /**
     * @brief Returns the internal QTreeView.
     */
    [[nodiscard]] auto treeView() const -> QTreeView*;

    /**
     * @brief Returns the internal StyledFilterBar.
     */
    [[nodiscard]] auto filterBar() const -> StyledFilterBar*;

    /**
     * @brief Sets the placeholder text of the search field.
     */
    void setSearchPlaceholder(const QString& text) const;

    /**
     * @brief Sets the filter combo box options.
     */
    void setFilterOptions(const QStringList& options) const;

    /**
     * @brief Sets the current search text.
     */
    void setSearchText(const QString& text) const;

signals:
    /**
     * @brief Emitted when the search text changes.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter combo index changes.
     */
    void filterIndexChanged(int index);

private:
    /**
     * @brief Builds the widget layout and connects filter signals.
     */
    void setupUi();

private:
    StyledFilterBar* m_filterBar = nullptr;
    QTreeView* m_treeView = nullptr;
};

} // namespace Core
