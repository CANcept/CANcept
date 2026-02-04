//
// Created by Adrian Rupp on 04.02.26.
//
#pragma once
#include <QStyledItemDelegate>
namespace DbcFile {
/**
 * @class MessagesDetailDelegate
 * @brief Renders detailed Signal cards in the bottom pane of the Messages Page.
 *
 * @details
 * Used in the Detail View (QListView) when a message is selected.
 * Displays all technical signal attributes (StartBit, Length, Factor, etc.) in a grid layout.
 */
class MessagesDetailDelegate : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit MessagesDetailDelegate(QObject* parent = nullptr);
    ~MessagesDetailDelegate() override = default;

    /**
     * @brief Paints a complex grid layout for technical signal attributes.
     *
     * @caller Qt View (QListView) during paint events.
     *
     * @details
     * Retrieves specific data using Custom Roles (`Role_StartBit`, `Role_Factor`, etc.)
     * regardless of column indices.
     * 1. Draws a card border.
     * 2. Header: Name + Unit.
     * 3. Grid: Draws labels ("Start Bit") and values ("0") in a structured layout.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Returns a large fixed height.
     * @caller Qt View layout system.
     * @return Height sufficient to fit the header, grid, and footer (~120px).
     */
    auto sizeHint(const QStyleOptionViewItem& option,
                  const QModelIndex& index) const -> QSize override;

private:
    void drawGridItem(QPainter* painter, const QRect& rect, const QString& label,
                      const QString& value) const;
    void drawBadge(QPainter* painter, const QRect& rect, const QString& text) const;
};
}
