//
// Created by Adrian Rupp on 30.12.25.
//
#pragma once

#include <QEvent>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTreeView>

namespace DbcFile {

/**
 * @class SidebarDelegate
 * @brief Simple delegate for the navigation sidebar to handle font weight and item height.
 *
 * @details
 * Since QSS (Stylesheets) often fail to apply font-weight changes on selection in QListViews,
 * this delegate manually sets the font to Bold when an item is selected.
 * It also enforces a fixed minimum height to ensure comfortable touch/click targets.
 */
class SidebarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit SidebarDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    /**
     * @brief Prepares the style option before painting.
     * @details Sets the font weight to Bold if the item is selected.
     */
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

    /**
     * @brief Defines the size of the sidebar items.
     * @details Adds vertical padding by enforcing a minimum height (e.g., 40px).
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;
};
// ==============================================================================
// 1. SidebarDelegate
// ==============================================================================

/**
 * @class SidebarDelegate
 * @brief Delegate for the navigation sidebar to control font weight and item height.
 *
 * @details
 * - Applies bold font for selected items (QSS cannot reliably change font-weight in QListView).
 * - Enforces a minimum item height to ensure consistent spacing and easy clicking/tapping.
 */
class SidebarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit SidebarDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    /**
     * @brief Prepares the style option before painting an item.
     *
     * @details
     * - Sets the font weight to Bold if the item is selected (QSS cannot reliably change
     * font-weight in QListView).
     * - Adjusts the icon color based on selection state:
     *      - Selected: Primary text color
     *      - Unselected: Secondary text color
     * - Ensures proper rendering of the item’s icon and text.
     *
     * @param option The style options for the item.
     * @param index The model index of the item being drawn.
     */
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

    /**
     * @brief Returns the preferred size for each sidebar item.
     * @details Adds vertical padding by enforcing a minimum height.
     *
     * @param option Style options containing font metrics.
     * @param index The model index of the item.
     * @return QSize The calculated item size, enforcing a comfortable minimum height (~50px).
     */
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;

    /**
     * @brief Shows a tooltip for disabled items.
     * @details Displays a static hover message (e.g., "Load DBC file first") when hovering
     *          over items that are currently disabled.
     *
     * @param event The QHelpEvent triggering the tooltip.
     * @param view The parent view containing the item.
     * @param option Style options for the item.
     * @param index The model index of the item.
     * @return True if a tooltip was shown, otherwise false.
     */
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;
};
// ==============================================================================
// 2. Overview Lists Delegate (Overview Page)
// ==============================================================================

/**
 * @class OverviewListsDelegate
 * @brief Renders items in the ECU and Message overview lists as "Cards".
 *
 * @details
 * Used in the QListViews on the Overview Page.
 * Draws a rounded rectangle containing an icon, the name, and a statistic badge.
 */
class OverviewListsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit OverviewListsDelegate(QObject* parent = nullptr);
    ~OverviewListsDelegate() override = default;

    /**
     * @brief Paints a rounded card with an icon and a count badge.
     *
     * @caller Qt View (QListView) during paint events.
     *
     * @details
     * 1. Draws a rounded rectangle background.
     * 2. Draws a placeholder icon on the left.
     * 3. Draws the item name (DisplayRole) in the center.
     * 4. Fetches `Role_MsgChildCount` (for ECUs) or `Role_ChildCount` (Messages)
     *    and draws it inside a gray badge on the right.
     *
     * @param painter The painter object used for drawing.
     * @param option Contains geometry (rect) and state (selected/hovered).
     * @param index The model index providing the data.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Determines the size of the card.
     *
     * @caller Qt View layout system.
     * @return A fixed size (e.g., 200x50px) to ensure proper grid alignment.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// ==============================================================================
// 3. ECU Tree Delegate (ECU Page)
// ==============================================================================

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

// ==============================================================================
// 4. Message Signal Card Delegate (Message Detail Pane)
// ==============================================================================

/**
 * @class MessagesSignalCardDelegate
 * @brief Renders detailed Signal cards in the bottom pane of the Messages Page.
 *
 * @details
 * Used in the Detail View (QListView) when a message is selected.
 * Displays all technical signal attributes (StartBit, Length, Factor, etc.) in a grid layout.
 */
class MessagesSignalCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit MessagesSignalCardDelegate(QObject* parent = nullptr);
    ~MessagesSignalCardDelegate() override = default;

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

// ==============================================================================
// 5. Signal Table Delegate (Global Signals Page)
// ==============================================================================

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

}  // namespace DbcFile