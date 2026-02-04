//
// Created by Adrian Rupp on 04.02.26.
//
#pragma once
#include <QStyledItemDelegate>
namespace DbcFile {
/**
 * @class SignalTableDelegate
 * @brief Renders cells for the global Signals Table.
 *
 * @details
 * Used in the main Signals Page (QTableView).
 * Provides custom rendering for specific columns (like Message ID badges) while
 * maintaining standard text rendering for others.
 */
class SignalTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit SignalTableDelegate(QObject* parent = nullptr);
    ~SignalTableDelegate() override = default;

    /**
     * @brief Renders the cell content with special handling for the Message Column.
     *
     * @caller Qt View (QTableView) during paint events.
     *
     * @details
     * 1. Checks if the column corresponds to the "Message" column.
     * 2. If yes: Draws a gray badge with the Message ID (`Role_Id`) and the Message Name.
     * 3. If no: Falls back to standard text rendering (optionally formatting hex/units via
     * `initStyleOption` logic if implemented).
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Ensures rows are tall enough for badges.
     * @caller Qt View layout system.
     * @return Height to fit the ID badge (usually slightly taller than standard text).
     */
    auto sizeHint(const QStyleOptionViewItem& option,
                  const QModelIndex& index) const -> QSize override;
};
}
