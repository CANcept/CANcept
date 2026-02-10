//
// Created by Adrian Rupp on 20.01.26.
//

#pragma once
#include <QLineEdit>
#include <QComboBox>

namespace Core {

/**
 * @brief A QWidget providing a combined search field and filter combo box.
 *
 * This widget is designed for tables, trees, or any list-based UI,
 * providing:
 * - A QLineEdit with an integrated search icon
 * - A QComboBox for filtering options
 * - Signals for text changes and filter selection changes
 *
 * Styling is applied automatically according to THEME constants.
 */
class StyledFilterBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param parent Parent widget
     */
    explicit StyledFilterBar(QWidget* parent = nullptr);

    /// @brief Returns the current text in the search field.
    [[nodiscard]] QString searchText() const;

    /// @brief Returns the currently selected filter text.
    [[nodiscard]] QString currentFilter() const;

    /// @brief Sets the placeholder text for the search field.
    void setPlaceholderText(const QString& text);

    /// @brief Sets the text in the search field programmatically.
    void setSearchText(const QString& text);

    /// @brief Sets the filter options available in the combo box.
    void setFilterOptions(const QStringList& options);

    /// @brief Sets the current filter text programmatically.
    void setCurrentFilter(const QString& text);

    /// @brief Sets the current filter by index.
    void setCurrentFilterIndex(int index);

signals:
    /// @brief Emitted whenever the search text changes.
    void searchTextChanged(const QString& text);

    /// @brief Emitted whenever the filter index changes.
    void filterIndexChanged(int index);

    /// @brief Emitted whenever the filter text changes.
    void filterOptionChanged(const QString& text);

private:
    /// @brief Sets up the UI elements (QLineEdit, QComboBox, layouts).
    void setupUi();

    /// @brief Applies THEME-based styles to the search field and combo box.
    void setupStyles();

    QLineEdit* m_searchBar = nullptr; ///< Internal search field
    QComboBox* m_filterBox = nullptr; ///< Internal filter combo box
};

} // namespace Core