#pragma once

#include <QCheckBox>

namespace Core {

/**
 * @class StyledCheckBox
 * @brief A customized QCheckBox widget with modern styling and custom painting.
 */
class StyledCheckBox final : public QCheckBox
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new Styled CheckBox.
     * @param parent The parent widget
     */
    explicit StyledCheckBox(QWidget* parent = nullptr);

    /**
     * @brief Constructs a new Styled CheckBox with text label.
     * @param text The label text to display next to the checkbox.
     * @param parent The parent widget
     */
    explicit StyledCheckBox(const QString& text, QWidget* parent = nullptr);

   protected:
    bool event(QEvent* event) override;

   private:
    /**
     * @brief Applies the application theme via Qt Stylesheets.
     */
    void applyStyle();

    bool m_hovered = false;
};

}  // namespace Core
