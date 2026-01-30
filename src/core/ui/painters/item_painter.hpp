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
     * @brief Paints the background of a Card item.
     *
     * Draws a rounded rectangle with color depending on selection state.
     * @param painter QPainter used for drawing.
     * @param rect The rectangle area of the card.
     * @param selected True if the card is selected, false otherwise.
     */
    static void paintCardBackground(QPainter* painter, const QRect& rect, bool selected);

    /**
     * @brief Paints an icon inside the Card item.
     * @param painter QPainter used for drawing.
     * @param rect Rectangle representing the card area.
     * @param icon Icon to draw.
     * @param selected Whether the icon should be drawn in selected state.
     *
     * The icon is vertically centered and tinted depending on selection state.
     */
    static void paintIcon(QPainter* painter, const QRect& rect, const QIcon& icon, bool selected);

    /**
     * @brief Paints the title text of a Card item.
     * @param painter QPainter used for drawing.
     * @param rect Rectangle representing the card area.
     * @param text The title text to display.
     * @param bold Whether the title should be drawn in bold.
     *
     * Text is truncated with "..." if it does not fit.
     */
    static void paintTitle(QPainter* painter, const QRect& rect, const QString& text,
                           bool bold = false);


    /**
     * @brief Paints a badge with optional icon and text inside the Card item.
     * @param painter QPainter used for drawing.
     * @param rect Rectangle representing the card area.
     * @param text Badge text.
     * @param icon Optional badge icon.
     * @return Width of the painted badge in pixels.
     *
     * Badge is drawn with a background, optional icon, and text.
     * Returns the badge width for layout adjustments (e.g., detail text positioning).
     */
    static auto paintBadge(QPainter* painter, const QRect& rect, const QString& text,
                           const QIcon& icon) -> int;

    /**
 * @brief Paints detail text to the right of the Card item.
 * @param p QPainter used for drawing.
 * @param rect Rectangle representing the card area.
 * @param text The detail text.
 * @param badgeWidth Width of the badge to avoid overlapping it.
 *
 * Ensures that the detail text does not overlap with the badge.
 */
    static void paintDetailText(QPainter* p, const QRect& rect, const QString& text, int badgeWidth);
};
}  // namespace Core
