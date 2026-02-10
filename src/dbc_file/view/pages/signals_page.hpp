#pragma once
#include "../../../core/widgets/common/searchable_filter_widgets.hpp"
namespace DbcFile {
/**
 * @class SignalsPage
 * @brief The page displaying a flat list of signals (SRS 6.5).
 *
 * @details
 * **VISUALS:**
 * - Top: Search Bar + Filter ComboBox (e.g. "All Units").
 * - Body: A table listing the signals matching search and filter.
 *
 * **LOGIC:**
 * Wraps a SearchableFilterTable and forwards user interaction signals.
 */
class SignalsPage : public QWidget
{
    Q_OBJECT

   public:
    explicit SignalsPage(QWidget* parent = nullptr);
    void populateUnitFilter(const QAbstractItemModel* model) const;
    ~SignalsPage() override = default;

    /**
     * @brief Sets the model for the table view.
     * @caller DbcView::setSourceModel().
     */
    void setModel(QAbstractItemModel* model) const;

    // /**
    //  * @brief Access to the filter combo to populate unit filters (e.g. "rpm", "km/h").
    //  * @caller DbcView::createSubViews().
    //  */
    // [[nodiscard]] auto getFilterCombo() const -> QComboBox*;

   signals:
    /**
     * @brief Emitted when the user types in the search bar.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the user changes the Unit filter dropdown.
     * @caller Internal SearchableFilterTable signal forwarding.
     */
    void filterUnitChanged(const QString& unit);

   private:
    /**
     * @brief Initializes the page layout.
     * @caller Constructor.
     * @details Instantiates the SearchableFilterTable.
     */
    void setupUi();

    Core::SearchableFilterTable* m_tableWidget;
};
}  // namespace DbcFile
