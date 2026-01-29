//
// Created by Adrian Rupp on 29.01.26.
//
#pragma once

class QString;
class QIcon;
class QRect;
class QPainter;
namespace Core {
/**
 * @class ItemPainter
 * @brief A static helper class for consistent rendering of custom UI items.
 *
 * @details
 * This class centralizes the painting logic for "Card" style list items used throughout
 * the application (e.g., in DBC Overview, Logging List, etc.).
 * It ensures that borders, colors, icons, and badges look identical everywhere.
 */
class ItemPainter
{
   public:
    ItemPainter() = delete;

    /**
     * @brief Draws the standard card background (rounded rect with border).
     *
     * @param painter The active painter.
     * @param rect The bounding rectangle of the item.
     * @param selected True if the item is currently selected/highlighted.
     */
    static void paintCardBackground(QPainter* painter, const QRect& rect, bool selected);

    /**
     * @brief Draws a tinted icon on the left side of the card.
     *
     * @param painter The active painter.
     * @param rect The item's bounding rect (used for relative positioning).
     * @param icon The icon to draw.
     * @param selected Determines the tint color (e.g., Primary vs Secondary).
     */
    static void paintIcon(QPainter* painter, const QRect& rect, const QIcon& icon, bool selected);

    /**
     * @brief Draws the main title text of the card.
     *
     * @param painter The active painter.
     * @param rect The item's bounding rect.
     * @param text The text to display.
     * @param bold If true, uses a bold font weight (e.g. for ECUs).
     */
    static void paintTitle(QPainter* painter, const QRect& rect, const QString& text,
                           bool bold = false);

    /**
     * @brief Draws a status badge (pill) on the right side of the card.
     *
     * @param painter The active painter.
     * @param rect The item's bounding rect.
     * @param text The text inside the badge (e.g. "4", "0x123").
     * @param icon Optional icon inside the badge.
     */
    static void paintBadge(QPainter* painter, const QRect& rect, const QString& text,
                           const QIcon& icon);
};
}  // namespace Core
