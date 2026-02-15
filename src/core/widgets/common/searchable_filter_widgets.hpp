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

#include <QFrame>
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
     * @brief Applies theme-dependent styling to the table and frame.
     *
     * The outer frame is responsible for rendering background,
     * border, and rounded corners.
     *
     * The internal QTableView, viewport, and header are rendered
     * fully transparent to prevent them from painting over the
     * frame's rounded edges.
     *
     * This method is re-invoked on theme changes via StyleEvent.
     */
    void applyStyle();

    /**
     * @brief Configures structural and behavioral table settings.
     *
     * Applies non-visual configuration such as:
     * - Scrollbar policies
     * - Grid visibility
     * - Header visibility
     * - Frame removal
     * - Background transparency attributes
     *
     * This method intentionally contains no styling logic.
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
     * @brief Builds and initializes the widget UI structure.
     *
     * Creates the main layout, filter bar, outer frame container,
     * and the internal QTableView. No styling logic is applied here.
     *
     * Styling is handled exclusively in applyStyle() to ensure
     * proper theme reactivity.
     */
    void setupUi();

    /**
     * @brief Handles custom StyleEvent for dynamic theme updates.
     *
     * When a StyleEvent is received, the widget reapplies its
     * stylesheet to reflect updated theme colors and spacing.
     */
    bool event(QEvent* event) override;

   private:
    StyledFilterBar* m_filterBar = nullptr;
    QTableView* m_tableView = nullptr;
    QFrame* m_tableFrame = nullptr;
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

}  // namespace Core
