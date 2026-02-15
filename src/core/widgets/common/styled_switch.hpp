#pragma once

#include <QAbstractButton>

namespace Core {

/**
 * @class StyledSwitch
 * @brief A modern toggle switch widget with theme-aware styling.
 */
class StyledSwitch final : public QAbstractButton
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new Styled Switch.
     * @param parent The parent widget
     */
    explicit StyledSwitch(QWidget* parent = nullptr);

    /**
     * @brief Returns the recommended size for the switch.
     * @return QSize with width and height optimized for the switch.
     */
    [[nodiscard]] QSize sizeHint() const override;

   protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool event(QEvent* event) override;

   private:
    /**
     * @brief Updates cached theme colors when theme changes.
     */
    void updateThemeColors();

    bool m_hovered = false;

    // Cached theme colors
    QColor m_trackColorOff;
    QColor m_trackColorOn;
    QColor m_thumbColor;
    QColor m_hoverColor;
};

}  // namespace Core
