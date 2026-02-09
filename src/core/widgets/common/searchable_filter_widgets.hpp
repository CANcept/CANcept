//
// Created by Adrian Rupp on 30.12.25.
//
#pragma once

#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QTreeView>

#include "core/widgets/common/styled_filter_bar.hpp"

namespace Core {
class StyledFilterBar;
}
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
 * Layout:
 * [ Search Bar ......... ] [ Filter Combo ]
 * [ Table View                            ]
 *
 * Used for lists that require flat data representation (Messages, Signals).
 */
class SearchableFilterTable final : public QWidget
{
    Q_OBJECT

   public:
    /**
 * @brief Constructor
 * @param parent Parent widget
 */
    explicit SearchableFilterTable(QWidget* parent = nullptr);
    ~SearchableFilterTable() override = default;

    /**
     * @brief Returns the internal table view.
     * @caller Parent Page (to set models/delegates).
     */
    [[nodiscard]] auto tableView() const -> QTableView*;

    /**
 * @brief Returns the internal filter bar widget.
 * @return Pointer to StyledFilterBar
 */
    [[nodiscard]] auto filterBar() const -> Core::StyledFilterBar*;

    /**
 * @brief Sets the placeholder text displayed in the search bar.
 * @param text Placeholder string
 */
    void setSearchPlaceholder(const QString& text) const;

    /**
 * @brief Sets the available filter options in the filter bar.
 * @param options List of filter option strings
 */
    void setFilterOptions(const QStringList& options) const;

    /**
 * @brief Sets the search text programmatically.
 * @param text Search string
 */
    void setSearchText(const QString& text) const;



   signals:
    /**
     * @brief Emitted when the search text changes.
     * @param text Current search string
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the selected filter index changes.
     * @param index Current filter index
     */
    void filterIndexChanged(const int index);

   private:
    /**
     * @brief Initializes layout and connections.
     * @caller Constructor.
     */
    void setupUi();


    QTableView* m_tableView = nullptr; ///< Internal table view
    Core::StyledFilterBar* m_filterBar = nullptr; ///< Filter bar
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
    /**
 * @brief Constructor
 * @param parent Parent widget
 */
    explicit SearchableFilterTree(QWidget* parent = nullptr);
    ~SearchableFilterTree() override = default;


    /**
 * @brief Returns the internal tree view.
 * @return Pointer to QTreeView
 */
    [[nodiscard]] auto treeView() const -> QTreeView*;


    /**
     * @brief Returns the internal filter bar widget.
     * @return Pointer to StyledFilterBar
     */
    [[nodiscard]] auto filterBar() const -> Core::StyledFilterBar*;

    /**
 * @brief Sets the placeholder text displayed in the search bar.
 * @param text Placeholder string
 */
    void setSearchPlaceholder(const QString& text) const;

    /**
 * @brief Sets the available filter options in the filter bar.
 * @param options List of filter option strings
 */
    void setFilterOptions(const QStringList& options) const;


    /**
     * @brief Sets the search text programmatically.
     * @param text Search string
     */
    void setSearchText(const QString& text) const;


   signals:
    /**
     * @brief Emitted when the search text changes.
     * @param text Current search string
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the selected filter index changes.
     * @param index Current filter index
     */
    void filterIndexChanged(int index);

   private:
    /**
     * @brief Initializes layout and connections.
     * @caller Constructor.
     */
    void setupUi();


    QTreeView* m_treeView = nullptr;///< Internal tree view
    Core::StyledFilterBar* m_filterBar = nullptr;///< Filter bar
};

}  // namespace DbcFile