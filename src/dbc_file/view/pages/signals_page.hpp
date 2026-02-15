#pragma once

#include <QStringList>
#include <QWidget>

class QAbstractItemModel;
class QTableView;

namespace Core {
class SearchableFilterTable;
}

namespace DbcFile {

/**
 * @brief The SignalsPage class represents the UI page displaying DBC signal data.
 *
 * This widget encapsulates:
 * - A styled header card
 * - A searchable and filterable table view
 * - Model binding and column configuration
 *
 * The class separates pure UI setup from model-dependent configuration
 * to maintain clean architecture and extensibility.
 */
class SignalsPage : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the SignalsPage.
     *
     * Initializes the UI structure and configures the table appearance.
     *
     * @param parent Optional parent widget.
     */
    explicit SignalsPage(QWidget* parent = nullptr);

    /**
     * @brief Sets the data model for the table view.
     *
     * Applies the given model to the internal table and configures
     * column visibility, sizing, and stretch behavior.
     *
     * @param model The model containing signal data.
     */
    void setModel(QAbstractItemModel* model) const;

    /**
     * @brief Updates the available unit filter options.
     *
     * Rebuilds the filter dropdown while preserving the previously
     * selected filter if possible.
     *
     * @param units List of available signal units.
     */
    void setAvailableUnits(const QStringList& units) const;

   signals:
    /**
     * @brief Emitted when the search text changes.
     *
     * @param text The current search string.
     */
    void filterTextChanged(const QString& text);

    /**
     * @brief Emitted when the unit filter changes.
     *
     * The emitted string represents the selected unit.
     * An empty string indicates "All".
     *
     * @param unit Selected unit.
     */
    void filterUnitChanged(const QString& unit);

   private slots:
    /**
     * @brief Internal slot handling filter index changes.
     *
     * Converts the selected filter entry into a unit string
     * and forwards it via filterUnitChanged().
     *
     * @param index The newly selected index.
     */
    void onFilterIndexChanged(int index);

   private:
    // =========================================================================
    // UI Setup
    // =========================================================================

    /**
     * @brief Creates and initializes the full UI layout.
     *
     * Builds the card container, search/filter table and connects signals.
     */
    void setupUi();

    /**
     * @brief Applies general table configuration independent of the model.
     *
     * Configures scrollbars, selection behavior, grid visibility
     * and vertical header.
     *
     * @param table Target table view.
     */
    static void configureTableBasics(QTableView* table);

    /**
     * @brief Applies visual styling to the table.
     *
     * Sets border radius, background colors and theme-based styling.
     *
     * @param table Target table view.
     */
    static void applyTableStyle(QTableView* table);

    /**
     * @brief Configures the horizontal header appearance.
     *
     * Applies resize mode and custom stylesheet styling.
     *
     * @param table Target table view.
     */
    static void configureHeaderStyle(const QTableView* table);

    /**
     * @brief Configures column visibility, width and stretch behavior.
     *
     * This method is model-dependent and should be called after
     * setting the model.
     *
     * @param table Target table view.
     * @param model Active model.
     */
    static void configureColumns(QTableView* table, const QAbstractItemModel* model);

   private:
    /// Searchable and filterable table widget
    Core::SearchableFilterTable* m_tableWidget{nullptr};
};

}  // namespace DbcFile