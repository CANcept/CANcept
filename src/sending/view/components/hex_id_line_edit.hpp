#pragma once

#include "core/widgets/styled_line_edit.hpp"

namespace Sending {

/**
 * @class HexIdLineEdit
 * @brief A specialized line edit for hexadecimal CAN ID input with "0x" prefix decoration.
 * - Displays "0x" prefix as visual decoration
 * - Automatic uppercase conversion
 * - Validates against standard CAN ID range (0x000 - 0x7FF)
 */
class HexIdLineEdit final : public Core::StyledLineEdit
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new hex ID line edit with standard CAN ID validation.
     * @param parent The parent widget
     */
    explicit HexIdLineEdit(QWidget* parent = nullptr);

    /**
     * @brief Sets the maximum allowed hex value for the CAN ID.
     * @param maxValue Maximum value (default is 0x7FF for standard CAN)
     */
    void setMaxHexValue(uint32_t maxValue);

   protected:
    /**
     * @brief Custom paint event to draw the "0x" prefix.
     */
    void paintEvent(QPaintEvent* event) override;

   private:
    void updateTextMargins();
};

}  // namespace Sending
