#pragma once

#include <QRect>
#include <QStyledItemDelegate>
#include <QTreeView>

namespace DbcFile {

/**
 * @brief Delegate class for rendering ECU and Message items in a QTreeView.
 *
 * This class provides custom painting and layout logic for ECU and CAN message cards.
 * It handles:
 * - Drawing ECU cards with icon, name, and badges for signal/message count.
 * - Drawing Message cards with headers, ID badge, and a grid layout for signals.
 * - Calculating sizes and positions for all card elements.
 *
 * The delegate uses THEME spacing and colors to maintain a consistent look.
 */
class EcuTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new EcuTreeDelegate.
     * @param view Pointer to the QTreeView that this delegate will render for.
     * @param parent Optional parent QObject.
     */
    explicit EcuTreeDelegate(QTreeView* view, QObject* parent = nullptr);
    ~EcuTreeDelegate() override = default;

    /**
     * @brief Returns the preferred size of an item.
     * @param option The style options for the item.
     * @param index The model index of the item.
     * @return The preferred QSize for the item.
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;

    /**
     * @brief Paints a given item.
     * @param painter The QPainter used for drawing.
     * @param option The style options for the item.
     * @param index The model index of the item.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

   private:
    /**
     * @brief Calculates the rectangle of a card, considering tree indentation.
     * @param option Style options.
     * @param indentLevel Level of indentation in the tree.
     * @return QRect adjusted for tree indentation and spacing.
     */
    [[nodiscard]] auto getCardRect(const QStyleOptionViewItem& option,
                                   int indentLevel) const -> QRect;

    // --- Layout Helper Struct ---
    struct MessageLayout {
        QRect cardRect;
        QRect headerRect;
        QRect contentRect;
        int columns = 1;
        int itemWidth = 0;
        int itemHeight = 0;
        int totalHeight = 0;
    };

    /**
     * @brief Calculates the layout of a message card, including header and signal grid.
     * @param fullRect The full area available for the card.
     * @param signalCount Number of signals to layout inside the message card.
     * @return A MessageLayout struct containing positions and sizes of elements.
     */
    [[nodiscard]] static auto calculateLayout(const QRect& fullRect,
                                              int signalCount) -> MessageLayout;

    /**
     * @brief Returns the row rectangle corresponding to the viewport width.
     * @param option Style options.
     * @return QRect covering the full viewport width.
     */
    [[nodiscard]] auto getViewportRowRect(const QStyleOptionViewItem& option) const -> QRect;

    /**
     * @brief Draws an ECU card with icon, title, and badges for signal/message counts.
     * @param painter The QPainter used for drawing.
     * @param option The style options for the item.
     * @param index The model index representing the ECU item.
     *
     * This method handles:
     * - Drawing the card background (highlighted if selected)
     * - Rendering the ECU icon and title
     * - Rendering badges for the number of messages and signals
     */
    void drawEcu(QPainter* painter, const QStyleOptionViewItem& option,
                 const QModelIndex& index) const;

    /**
     * @brief Draws a message card, including its header and a grid of signal items.
     * @param painter The QPainter used for drawing.
     * @param option The style options for the item.
     * @param index The model index representing the Message item.
     *
     * This method calculates the layout based on the number of child signals
     * and delegates the drawing of the header and each signal item.
     */
    void drawMessage(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const;

    /**
     * @brief Draws the header of a message card.
     * @param painter The QPainter used for drawing.
     * @param headerRect The rectangle representing the header area.
     * @param index The model index of the message.
     *
     * The header includes:
     * - Optional icon (DecorationRole)
     * - Message name (DisplayRole)
     * - ID badge (Role_Id)
     * - Signal count badge (Role_ChildCount)
     */
    static void drawMessageHeader(QPainter* painter, const QRect& headerRect,
                                  const QModelIndex& index);

    /**
     * @brief Draws a single signal item inside a message card.
     * @param painter The QPainter used for drawing.
     * @param rect Rectangle representing the area for this signal item.
     * @param sigIdx The model index of the signal.
     *
     * This method handles:
     * - Drawing the signal background as a mini card
     * - Rendering the signal name, value range, and unit badge
     * - Layout is calculated right-to-left for badges and left-to-right for the name
     */
    static void drawSignalItem(QPainter* painter, const QRect& rect, const QModelIndex& sigIdx);

    QTreeView* m_treeView;
};

}  // namespace DbcFile