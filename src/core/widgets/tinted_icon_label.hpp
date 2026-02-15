#pragma once

#include <QColor>
#include <QIcon>
#include <QLabel>

namespace Core {
/**
 * @class TintedIconLabel
 * @brief A QLabel extension that displays an SVG icon tinted with a specific color.
 *
 * @details
 * This widget automates the process of loading an icon (usually SVG), converting it
 * to a QPixmap, and applying a color overlay using `QPainter::CompositionMode_SourceIn`.
 * This ensures that icons always match the application's theme (e.g., primary text color),
 * regardless of the original SVG color.
 */
class TintedIconLabel : public QLabel
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the tinted icon label.
     *
     * @param iconPath Path to the resource file (e.g. ":/icons/chip.svg").
     * @param size The width/height of the icon in pixels (square).
     * @param color The color to apply to the icon content.
     * @param parent The parent widget.
     */
    explicit TintedIconLabel(const QString& iconPath, int size, const QColor& color,
                             QWidget* parent = nullptr);

    ~TintedIconLabel() override = default;

    /**
     * @brief Updates the tint color of the icon.
     * Use this to change appearance dynamically (e.g., on hover or theme change).
     */
    void setColor(const QColor& color);

    /**
     * @brief Loads a new icon from the given path and re-applies the current tint.
     */
    void setIconPath(const QString& path);

    /**
     * @brief Updates the target size of the icon.
     */
    void setIconSize(int size);

   private:
    /**
     * @brief Internal helper to redraw the pixmap with the current settings.
     */
    void updatePixmap();

    QString m_path;
    QColor m_color;
    int m_size;
};
}  // namespace Core