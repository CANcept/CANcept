#pragma once

#include <QPushButton>

namespace Sending {

/**
 * @class SendMessageButton
 * @brief Styled primary action button for sending CAN messages.
 *
 * This component provides a consistent styled button with proper theming,
 * hover effects, and icon alignment for the send message action.
 * Supports two states: normal (Send) and active (Stop).
 */
class SendMessageButton final : public QPushButton
{
    Q_OBJECT

   public:
    explicit SendMessageButton(QWidget* parent = nullptr);
    ~SendMessageButton() override = default;

    /**
     * @brief Sets the button to sending state.
     * @param sending true for sending state, false for normal state
     */
    void setSendingState(bool sending);

    /**
     * @brief Returns whether the button is in sending state.
     */
    [[nodiscard]] auto isSendingState() const -> bool
    {
        return m_isSending;
    }

   protected:
    bool event(QEvent* event) override;

   private:
    void applyStyle();
    void updateAppearance();

    bool m_isSending = false;
};

}  // namespace Sending
