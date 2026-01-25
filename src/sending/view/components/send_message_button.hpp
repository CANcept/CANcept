#pragma once

#include <QPushButton>

namespace Sending {

/**
 * @class SendMessageButton
 * @brief Styled primary action button for sending CAN messages.
 *
 * This component provides a consistent styled button with proper theming,
 * hover effects, and icon alignment for the send message action.
 */
class SendMessageButton final : public QPushButton
{
    Q_OBJECT

   public:
    explicit SendMessageButton(QWidget* parent = nullptr);
    ~SendMessageButton() override = default;

   private:
    void applyStyle();
};

}  // namespace Sending
