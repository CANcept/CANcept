#pragma once

#include <QComboBox>

namespace Core {

/**
 * @class StyledComboBox
 * @brief A styled combo box with rounded corners and theme-aware colors.
 *
 * Provides a modern combo box appearance that matches the application theme.
 */
class StyledComboBox : public QComboBox
{
    Q_OBJECT

   public:
    explicit StyledComboBox(QWidget* parent = nullptr);
    ~StyledComboBox() override = default;

   private:
    void applyStyle();
};

}  // namespace Core
