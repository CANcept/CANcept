#pragma once

#include <QLineEdit>

namespace Core {

/**
 * @class StyledLineEdit
 * @brief A customized QLineEdit widget that adheres to the application's visual theme.
 *
 * The StyledLineEdit wraps the standard Qt QLineEdit and automatically applies
 * the project's design system.
 */
class StyledLineEdit : public QLineEdit
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new Styled Line Edit.
     *
     * @param parent The parent widget (default is nullptr).
     */
    explicit StyledLineEdit(QWidget* parent = nullptr);

    /**
     * @brief Constructs a new Styled Line Edit with initial text.
     *
     * @param text The initial text to display in the line edit.
     * @param parent The parent widget (default is nullptr).
     */
    explicit StyledLineEdit(const QString& text, QWidget* parent = nullptr);

    /**
     * @brief Overrides the default theme padding for this instance.
     *
     * Must be called after construction. The values are preserved across theme switches
     * because applyStyle() reads them instead of the theme defaults.
     *
     * @param vertical   Top/bottom padding in pixels.
     * @param horizontal Left/right padding in pixels.
     */
    void setPadding(int vertical, int horizontal);

   protected:
    /**
     * @brief Custom paint event to draw high-fidelity borders.
     * Overridden to prevent corner overlap artifacts by using a single path.
     */
    void paintEvent(QPaintEvent* event) override;

    bool event(QEvent* event) override;

   private:
    /**
     * @brief Applies the application theme via Qt Stylesheets.
     *
     * This method fetches the current color palette and spacing constants
     * from the Core::Theme global instance and formats the QLineEdit
     * stylesheet string (CSS) accordingly.
     */
    void applyStyle();

    int m_paddingV{-1};
    int m_paddingH{-1};
};

}  // namespace Core