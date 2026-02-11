#pragma once
#include <QPainter>
#include <QIcon>

namespace Core {


/**
 * @brief Utility class for painting UI items such as cards, badges, icons, and text.
 *
 * This class provides standardized methods for rendering visual elements in the GUI
 * with proper spacing, colors, and theming. It is designed to work with QPainter.
 */
class ItemPainter
{
public:

    /**
* @brief Style for rendering a badge.
*/
    struct BadgeStyle {
        QColor background; ///< Background color of the badge.
        QColor text;       ///< Text and icon color.
        QColor border;       ///< Optional border pen.
    };

    /**
     * @brief Paints a rounded card rectangle.
     * @param painter The QPainter to draw with.
     * @param rect The rectangle representing the card area.
     * @param selected True if the card is selected; applies highlight style.
     */
    static void paintCard(QPainter* painter, const QRect& rect, bool selected);


    /**
* @brief Measures the size of a badge based on text and optional icon.
* @param text Text displayed inside the badge.
* @param icon Optional icon displayed inside the badge.
* @return QSize representing the width and height required to render the badge.
*/
    static auto measureBadge(const QString& text, const QIcon& icon = QIcon()) -> QSize;

    /**
 * @brief Paints a badge with text and optional icon.
 * @param painter The QPainter used for drawing.
 * @param rect Rectangle specifying the badge position and size.
 * @param text Text to display inside the badge.
 * @param icon Optional icon to display inside the badge.
 * @param style Optional custom style. If nullptr, a default style is applied.
 */
    static void paintBadge(QPainter* painter, const QRect& rect,
                           const QString& text, const QIcon& icon = QIcon(),
                           const BadgeStyle* style = nullptr);

    /**
 * @brief Paints an icon centered in the given rectangle.
 * @param painter The QPainter used for drawing.
 * @param rect Rectangle in which the icon should be centered.
 * @param icon The icon to render.
 * @param selected True if the item is selected; can affect color.
 */
    static void paintIcon(QPainter* painter, const QRect& rect, const QIcon& icon, bool selected);

    /**
 * @brief Paints text inside a rectangle with optional bold style and color.
 * @param painter The QPainter used for drawing.
 * @param rect Rectangle specifying the text area.
 * @param text Text to display.
 * @param bold True to draw the text bold.
 * @param color Optional color for the text; uses theme primary text if invalid.
 * @param align Alignment of the text inside the rectangle.
 * @param elide True to elide text if it exceeds the width of the rectangle.
 */
    static void paintText(QPainter* painter, const QRect& rect, const QString& text,
                          bool bold = false,
                          const QColor& color = QColor(), // <--- NEU
                          Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter,
                          bool elide = true);


    /**
     * @brief Paints a row background with subtle border line at the bottom.
     * @param painter The QPainter used for drawing.
     * @param rect Rectangle representing the row area.
     * @param alternate Optional parameter to indicate alternating row colors (unused).
     */
    static void paintRow(QPainter* painter, const QRect& rect, bool alternate);


private:
    /**
     * @brief Returns a default badge style using theme colors.
     * @return BadgeStyle with default background, text, and border colors.
     */
    static auto defaultBadgeStyle() -> BadgeStyle;
};
} // namespace Core