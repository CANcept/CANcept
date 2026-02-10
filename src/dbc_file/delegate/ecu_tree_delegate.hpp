#pragma once

#include <QPainter>
#include <QRect>
#include <QStyledItemDelegate>
#include <QTreeView>

namespace Core {
enum class DbcItemType;
}

/**
 * @brief Namespace for DBC-related UI components
 */
namespace DbcFile {

/**
 * @brief Layout information for a message card.
 *
 * Contains the rectangles and sizing info for the card, header, content,
 * and signal grid, as well as total height for proper sizeHint calculation.
 */
struct MessageLayout {
    QRect cardRect;       ///< Full card background rect
    QRect headerRect;     ///< Header area (name + badge)
    QRect contentRect;    ///< Area for signals
    int itemWidth = 0;    ///< Width of a single signal item
    int itemHeight = 0;   ///< Height of a single signal item
    int columns = 1;      ///< Number of columns in signal grid
    int totalHeight = 0;  ///< Total height including padding and margins
};

/**
 * @brief Delegate for rendering ECUs and Messages with signals in a QTreeView.
 *
 * Handles painting of:
 * - ECU items (with badges)
 * - Message items (card style, header, badges, signal grid)
 *
 * Also calculates layout and provides sizeHint for proper row height.
 */
class EcuTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    /**
     * @brief Construct a new delegate.
     * @param view The QTreeView this delegate belongs to
     * @param parent Optional parent object
     */
    explicit EcuTreeDelegate(QTreeView* view, QObject* parent = nullptr);

    // -------------------------------------------------------------
    // Standard QStyledItemDelegate overrides
    // -------------------------------------------------------------
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> QSize override;

   private:
    QTreeView* m_treeView = nullptr;  ///< Associated tree view

    // =============================================================
    // Layout helpers
    // =============================================================
    /**
     * @brief Calculates the layout for a message card.
     * @param fullRect Full row rect from QTreeView
     * @param signalCount Number of signals in this message
     * @return MessageLayout Calculated layout including header, content, and total height
     */
    static auto calculateLayout(const QRect& fullRect, int signalCount) -> MessageLayout;

    /**
     * @brief Gets the full row rect aligned to the tree viewport width
     */
    [[nodiscard]] auto getViewportRowRect(const QStyleOptionViewItem& option) const -> QRect;

    // =============================================================
    // Drawing helpers
    // =============================================================
    void drawMessage(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const;

    static void drawMessageHeader(QPainter* painter, const QRect& headerRect,
                                  const QModelIndex& index);

    static void drawSignalItem(QPainter* painter, const QRect& rect, const QModelIndex& sigIdx);

    void drawEcu(QPainter* painter, const QStyleOptionViewItem& option,
                 const QModelIndex& index) const;

    static void drawBadge(QPainter* painter, const QRect& anchorRect, const QString& text,
                          const QColor& bg, const QColor& fg, bool border = true);
};

}  // namespace DbcFile