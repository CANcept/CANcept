/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QWidget>

class QLineEdit;
class QComboBox;

namespace Core {

/**
 * @brief A styled filter bar widget combining a search field and a filter dropdown.
 *
 * StyledFilterBar provides:
 * - A search input field with leading search icon
 * - A filter combo box for category-based filtering
 * - Theme-based styling using THEME colors and spacing
 *
 * The widget emits signals when the search text or filter selection changes.
 */
class StyledFilterBar : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a StyledFilterBar.
     * @param parent Optional parent widget.
     */
    explicit StyledFilterBar(QWidget* parent = nullptr);
    ~StyledFilterBar() override = default;

    // -------------------------------------------------------------------------
    // Getter
    // -------------------------------------------------------------------------

    /**
     * @brief Returns the current search text.
     * @return Text entered the search field.
     */
    [[nodiscard]] auto searchText() const -> QString;

    /**
     * @brief Returns the currently selected filter text.
     * @return Display text of the selected filter item.
     */
    [[nodiscard]] auto currentFilterText() const -> QString;

    /**
     * @brief Returns the user data of the currently selected filter item.
     * @return QVariant containing the user data.
     */
    [[nodiscard]] auto currentFilterData() const -> QVariant;

    // -------------------------------------------------------------------------
    // Setter / Configuration
    // -------------------------------------------------------------------------

    /**
     * @brief Sets the placeholder text of the search field.
     * @param text Placeholder string.
     */
    void setPlaceholderText(const QString& text);

    /**
     * @brief Sets the current search text.
     * @param text Text to apply to the search field.
     */
    void setSearchText(const QString& text);

    /**
     * @brief Removes all filter options from the combo box.
     */
    void clearFilterOptions();

    /**
     * @brief Sets filter options using a list of strings.
     * @param options List of filter display texts.
     *
     * Existing options are cleared before inserting the new ones.
     */
    void setFilterOptions(const QStringList& options);

    /**
     * @brief Adds a single filter option to the combo box.
     * @param text Display text of the filter option.
     * @param userData Optional user data associated with the item.
     */
    void addFilterOption(const QString& text, const QVariant& userData);

    /**
     * @brief Sets the currently selected filter by display text.
     * @param text Display text to select.
     *
     * Falls back to index 0 if the text is not found.
     */
    void setCurrentFilterText(const QString& text);

    /**
     * @brief Sets the currently selected filter by index.
     * @param index Index of the filter item.
     */
    void setCurrentFilterIndex(int index);

   signals:
    /**
     * @brief Emitted when the search text changes.
     * @param text Updated search string.
     */
    void searchTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter index changes.
     * @param index New index of the selected filter option.
     */
    void filterIndexChanged(int index);

   private:
    /**
     * @brief Initializes UI elements and layout.
     *
     * Creates the search field, filter combo box,
     * sets up layout and connects internal signals.
     */
    void setupUi();

    /**
     * @brief Applies theme-dependent styling to the search bar and filter combo box.
     */
    void applyStyle();
    /**
     * @brief Handles custom events for styling.
     *
     * Refreshes style for child widgets if a Core::StyleEvent is received.
     * @param event The event object.
     * @return True if the event was handled.
     */
    auto event(QEvent* event) -> bool override;

   private:
    QLineEdit* m_searchBar = nullptr;  ///< Search input field
    QComboBox* m_filterBox = nullptr;  ///< Filter dropdown box
};

}  // namespace Core