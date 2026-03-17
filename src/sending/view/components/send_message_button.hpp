/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
