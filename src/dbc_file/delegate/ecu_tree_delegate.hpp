//
// Created by Adrian Rupp on 04.02.26.
//
#pragma once
#include <QStyledItemDelegate>
#include <QTreeView>
namespace DbcFile {
/**
 * @class EcuTreeDelegate
 * @brief Renders the ECU Hierarchy content as cards.
 *
 * @details
 * Used in the QTreeView on the ECU Page.
 * - **Expansion:** Relies on the standard QTreeView expansion (arrow on the left).
 * - **Visualization:** Draws custom "Cards" next to the standard arrow/indentation.
 * - **Structure:** Differentiates visuals based on `Role_ItemType` (ECU, Message, Signal).
 */
class EcuTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the delegate.
     * @param view Reference to the TreeView (useful for checking indentation logic if needed).
     */
    explicit EcuTreeDelegate(QTreeView* view, QObject* parent = nullptr);
    ~EcuTreeDelegate() override = default;

    /**
     * @brief Renders the item content based on its DbcItemType.
     *
     * @caller Qt View (QTreeView) during paint events.
     *
     * @details
     * - **ECU:** Draws a large card frame, bold name, and Message Count Badge.
     * - **Message:** Draws a header-like row with an ID Badge.
     * - **Signal:** Draws a detail row with value range [min, max] and Unit Badge.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Calculates height based on Item Type.
     *
     * @caller Qt View layout system.
     * @return Larger height for ECUs (Header), smaller height for Signals (Detail).
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    // --- Painting Helpers ---
    void paintEcuCard(QPainter* painter, const QStyleOptionViewItem& option,
                      const QModelIndex& index);
    void paintMessageRow(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const;
    void paintSignalRow(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const;

    /** @brief Helper to draw a rounded gray badge with text. */
    void drawBadge(QPainter* painter, const QRect& rect, const QString& text, const QColor& bg,
                   const QColor& fg) const;

    QTreeView* m_treeView;
};
}  // namespace DbcFile