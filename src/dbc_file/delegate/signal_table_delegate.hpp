#pragma once

#include <QModelIndex>
#include <QPainter>
#include <QStyledItemDelegate>

namespace DbcFile {

/**
 * @brief Delegate for painting signal table rows with custom formatting.
 *
 * SignalTableDelegate renders each row in a QTableView representing CAN signals.
 * It customizes the appearance of:
 * - Message column (ID badge + name)
 * - Range column ([min, max])
 * - Length column (with optional unit)
 * - Unit column
 * - Standard text columns (with bolding for the signal name)
 *
 * The delegate uses Core::ItemPainter for consistent painting according to the theme.
 */
class SignalTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a SignalTableDelegate.
     * @param parent Optional parent QObject.
     */
    explicit SignalTableDelegate(QObject* parent = nullptr);

    /**
     * @brief Paints a cell in the signal table.
     * @param painter Painter object to perform drawing.
     * @param option Style options for the cell.
     * @param index Model index identifying the cell.
     *
     * Painting behavior depends on the column:
     * - Message column: ID badge + signal name
     * - Range column: "[min, max]" formatted text
     * - Length column: value with optional unit
     * - Unit column: default or model-provided unit
     * - Other columns: standard text (signal name bolded)
     *
     * Row backgrounds and separators are painted automatically.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Returns the preferred size of a table cell.
     * @param option Style options.
     * @param index Model index of the cell.
     * @return QSize representing cell width and height.
     *
     * Height is fixed using the application's theme spacing (HeightSm).
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;
};

}  // namespace DbcFile