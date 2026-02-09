#pragma once

#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QTreeView>

namespace DbcFile {

// ==============================================================================
// 1. Searchable Filter Table
// ==============================================================================

/**
 * @class SearchableFilterTable
 * @brief A reusable compound widget containing a Search Bar (top), Filter Combo (top), and Table
 * (bottom).
 *
 * @details
 * **Layout:**
 * [ Search Bar ......... ] [ Filter Combo ]
 * [ Table View                            ]
 *
 * Used for lists that require flat data representation (Messages, Signals).
 */
class SearchableFilterTable : public QWidget
{
    Q_OBJECT

   public:
    explicit SearchableFilterTable(QWidget* parent = nullptr);
    ~SearchableFilterTable() override = default;

    /**
     * @brief Returns the internal table view.
     * @caller Parent Page (to set models/delegates).
     */
    [[nodiscard]] auto tableView() const -> QTableView*;

    /**
     * @brief Returns the search line edit.
     * @caller Parent Page (e.g. to set placeholder text).
     */
    [[nodiscard]] auto searchBar() const -> QLineEdit*;

    /**
     * @brief Returns the filter combo box.
     * @caller Parent Page (to add items like "All Units").
     */
    [[nodiscard]] auto filterComboBox() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the text in the search bar changes.
     * @caller Internal QLineEdit signal.
     * @param text The current search string.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user selects a different item in the combo box.
     * @caller Internal QComboBox signal.
     * @param index The index of the selected item.
     */
    void filterTypeChanged(int index);

   private:
    /**
     * @brief Initializes layout and connections.
     * @caller Constructor.
     */
    void setupUi();

    QTableView* m_tableView;
    QLineEdit* m_searchBar;
    QComboBox* m_filterCombo;
};

// ==============================================================================
// 2. Searchable Filter Tree
// ==============================================================================

/**
 * @class SearchableFilterTree
 * @brief A reusable compound widget containing a Search Bar (top), Filter Combo (top), and Tree
 * (bottom).
 *
 * @details
 * **Layout:**
 * [ Search Bar ......... ] [ Filter Combo ]
 * [ Tree View                             ]
 *
 * Used for hierarchical data representation (ECUs).
 */
class SearchableFilterTree : public QWidget
{
    Q_OBJECT

   public:
    explicit SearchableFilterTree(QWidget* parent = nullptr);
    ~SearchableFilterTree() override = default;

    /**
     * @brief Returns the internal tree view.
     * @caller Parent Page (to set models/delegates).
     */
    [[nodiscard]] auto treeView() const -> QTreeView*;

    /**
     * @brief Returns the search line edit.
     */
    [[nodiscard]] auto searchBar() const -> QLineEdit*;

    /**
     * @brief Returns the filter combo box.
     */
    [[nodiscard]] auto filterComboBox() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the text in the search bar changes.
     * @caller Internal QLineEdit signal.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user selects a different item in the combo box.
     * @caller Internal QComboBox signal.
     */
    void filterTypeChanged(int index);

   private:
    /**
     * @brief Initializes layout and connections.
     * @caller Constructor.
     */
    void setupUi();

    QTreeView* m_treeView;
    QLineEdit* m_searchBar;
    QComboBox* m_filterCombo;
};

}  // namespace DbcFile