#pragma once

#include "core/widgets/common/searchable_filter_widgets.hpp"

class QTreeView;

namespace Core {
class CardWidget;
}

namespace DbcFile {

/**
 * @brief Widget representing the ECUs page.
 *
 * Contains:
 * - Header card (title + subtitle)
 * - Searchable/filterable tree of ECUs and messages
 * - Signal forwarding for search/filter changes
 */
class EcusPage : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Construct the ECU page.
     * @param parent Optional parent widget
     */
    explicit EcusPage(QWidget* parent = nullptr);

    /**
     * @brief Set the model displayed in the tree view.
     * @param model The model containing ECU/message data
     */
    void setModel(QAbstractItemModel* model) const;

    signals:
     /**
      * @brief Emitted when the search text changes in the filter bar.
      * @param text Current search string
      */
     void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the filter category changes.
     * @param index Filter index (integer)
     */
    void filterIndexChanged(int index);

private:
    /**
     * @brief Setup UI components and layout.
     */
    void setupUi();

private:
    Core::SearchableFilterTree* m_treeWidget;  ///< The searchable/filterable tree
};

}  // namespace DbcFile