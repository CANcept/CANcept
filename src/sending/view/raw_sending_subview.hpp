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

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "components/hex_id_line_edit.hpp"
#include "components/repeated_sending_card.hpp"
#include "components/send_message_button.hpp"
#include "core/widgets/card_widget.hpp"
#include "sending/view/formatter/hex_data_formatter.hpp"

namespace Sending {

/**
 * @class RawSendingSubView
 * @brief Modern card-based view for raw CAN frame composition.
 *
 * @section Layout Architecture
 * The view uses a card-based design matching the Figma mockups:
 * 1. CAN-Bus Configuration Card: Interface and Baud Rate selection
 * 2. CAN Frame Card: Single-line hex inputs for ID and Message Data
 * 3. Repeated Sending Card: Configure cyclic message transmission
 * 4. Floating Send Message button at bottom right
 */
class RawSendingSubView final : public QWidget
{
    Q_OBJECT

   public:
    explicit RawSendingSubView(QWidget* parent = nullptr);
    ~RawSendingSubView() override = default;

    /**
     * @name Frame Data Accessors
     * @{
     */
    [[nodiscard]] auto canIdEditor() const -> QLineEdit*
    {
        return m_canIdEditor;
    }
    [[nodiscard]] auto messageDataEditor() const -> QLineEdit*
    {
        return m_messageDataEditor;
    }
    /** @} */

    /**
     * @name Control Accessors
     * @{
     */
    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }

    [[nodiscard]] auto repeatedSendingCard() const -> RepeatedSendingCard*
    {
        return m_repeatedSendingCard;
    }
    /** @} */

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle();
    void setupCanIdInput() const;
    void setupMessageDataInput();

    // Scroll area
    QScrollArea* m_scrollArea;

    // CAN Frame Card
    Core::CardWidget* m_frameCard;
    HexIdLineEdit* m_canIdEditor;
    QLineEdit* m_messageDataEditor;
    HexDataFormatter* m_messageDataFormatter;
    QLabel* m_canIdLabel;
    QLabel* m_messageDataLabel;

    // Repeated Sending Card
    RepeatedSendingCard* m_repeatedSendingCard;

    // Floating Send Button
    QPushButton* m_sendButton;
};

}  // namespace Sending