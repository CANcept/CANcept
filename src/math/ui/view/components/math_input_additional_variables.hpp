#pragma once

#include "math_input_button.hpp"

namespace Math {

/**
 * @brief Smaller variant of MathInputButton that also draws a themed outline on hover.
 */
class MathInputAdditionalVariables final : public MathInputButton
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the button with an icon and optional parent widget.
     */
    explicit MathInputAdditionalVariables(const QString& iconPath, QWidget* parent = nullptr);

    /**
     * @brief Returns the preferred size, smaller than a standard MathInputButton.
     */
    [[nodiscard]] auto sizeHint() const -> QSize override;

   protected:
    void paintEvent(QPaintEvent* event) override;
    auto event(QEvent* event) -> bool override;

   private:
    QColor m_outlineColor;
};

}  // namespace Math
