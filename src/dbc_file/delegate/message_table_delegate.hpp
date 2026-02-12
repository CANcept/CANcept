/**
* @file message_table_delegate.hpp
 * @brief Delegate for rendering rows in the messages table view.
 *
 * MessageTableDelegate customizes the appearance of message rows:
 * - Draws themed row background (selected/hovered state).
 * - Renders the message ID as a centered badge in hex format.
 * - Centers text for the remaining columns.
 *
 * Data is obtained via Qt roles and custom DBC roles (see dbc_roles.hpp).
 */

#pragma once

#include <QStyledItemDelegate>

class QPainter;
class QStyleOptionViewItem;
class QModelIndex;
class QSize;

namespace DbcFile {

/**
 * @class MessageTableDelegate
 * @brief QStyledItemDelegate that paints message table cells with custom styling.
 */
class MessageTableDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the delegate.
     * @param parent Optional QObject parent.
     */
    explicit MessageTableDelegate(QObject* parent = nullptr);

    /**
     * @brief Paints a single table cell for a message row.
     * @param painter Painter provided by the view.
     * @param option Style option containing rect and state flags.
     * @param index Model index for the cell.
     *
     * The delegate draws the row background and then paints the column-specific content.
     * The message ID column is rendered as a badge; other columns are centered text.
     */
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Returns the size hint for a message row.
     * @param option Style option (width is taken from option.rect).
     * @param index Model index (unused for sizing).
     * @return Size hint with themed row height.
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const -> QSize override;
};

} // namespace DbcFile
