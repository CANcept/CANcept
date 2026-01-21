//
// Created by Adrian Rupp on 22.01.26.
//
#pragma once
#include <QWidget>

#include "dbc_file/view/searchable_filter_widgets.hpp"
namespace DbcFile {
/**
 * @class EcusPage
 * @brief The page displaying the ECU hierarchy.
 *
 * @details
 * **VISUALS:**
 * - Uses SearchableFilterTree (Search + Filter + Tree).
 * - Standard Qt Expansion (Arrow on the left).
 * - Custom Delegate (EcuTreeDelegate) used for Card-like rendering.
 *
 * **LOGIC:**
 * Acts as a wrapper around the tree view and forwards user input signals.
 */
class EcusPage : public QWidget
{
    Q_OBJECT
   public:
    explicit EcusPage(QWidget* parent = nullptr);
    ~EcusPage() override = default;

    /**
     * @brief Sets the source model for the internal tree view.
     * @caller DbcView::setSourceModel() to inject the TreeFilterProxy.
     * @param model The proxy model to be displayed.
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Access to the filter combo box to populate it with options.
     * @caller DbcView::createSubViews() to add items like "All ECUs".
     * @return Pointer to the internal QComboBox.
     */
    [[nodiscard]] auto getFilterCombo() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the user types text into the search bar.
     * @caller Internal SearchableFilterTree signal forwarding.
     * @param text The current text in the search field.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user selects a different option in the filter combo box.
     * @caller Internal SearchableFilterTree signal forwarding.
     * @param index The index of the selected combo box item.
     */
    void filterTypeChanged(int index);

   private:
    /**
     * @brief Initializes the page layout.
     * @caller Constructor.
     * @details Instantiates the SearchableFilterTree and installs the EcuTreeDelegate on the view.
     */
    void setupUi();

    SearchableFilterTree* m_treeWidget;
};
}  // namespace DbcFile