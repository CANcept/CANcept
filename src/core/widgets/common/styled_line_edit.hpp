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
};

}  // namespace Core