#pragma once

#include <QAbstractButton>
#include <QPixmap>

namespace Math {

/**
 * @brief Icon-sized styled button for inserting a single token into the expression.
 *
 */
class MathInputButton : public QAbstractButton
{
    Q_OBJECT

   public:
    /** @brief Tag to select text-label display mode. */
    struct AsLabel {
    };

    /**
     * @brief Constructs an icon-mode button from the given SVG asset path.
     */
    explicit MathInputButton(QString iconPath, QWidget* parent = nullptr);

    /**
     * @brief Constructs a text-label-mode button displaying the given string.
     */
    explicit MathInputButton(AsLabel, const QString& label, QWidget* parent = nullptr);

    /**
     * @brief Returns the preferred square size based on theme spacing.
     */
    [[nodiscard]] auto sizeHint() const -> QSize override;

   protected:
    void paintEvent(QPaintEvent* event) override;
    auto event(QEvent* event) -> bool override;

   private:
    void applyStyle();

    QString m_iconPath;
    QPixmap m_pixmap;
    QColor m_background;
    QColor m_hover;
    QColor m_textColor;
    bool m_hovered = false;
};

}  // namespace Math
