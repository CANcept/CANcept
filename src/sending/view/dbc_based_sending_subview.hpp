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

#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "components/repeated_sending_card.hpp"
#include "components/send_message_button.hpp"
#include "core/widgets/dbc_message_card.hpp"

namespace Core {
class CardWidget;
}

namespace Sending {

class SendingModel;

/**
 * @class DbcSendingSubView
 * @brief The primary container for the DBC-based workflow.
 */
class DbcSendingSubView final : public QWidget
{
    Q_OBJECT
   public:
    explicit DbcSendingSubView(QWidget* parent = nullptr);
    ~DbcSendingSubView() override = default;

    /**
     * @brief Populates the view by reading from the model.
     * Creates message cards and signal rows based on model data.
     * This maintains MVD separation - View reads Model, doesn't modify it.
     */
    void populateFromModel(const SendingModel* model);

    /**
     * @brief Clears all message cards.
     */
    void clearMessages() const;

    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }

    [[nodiscard]] auto repeatedSendingCard() const -> RepeatedSendingCard*
    {
        return m_repeatedSendingCard;
    }

   signals:
    /**
     * @brief Emitted when user toggles message selection checkbox.
     */
    void messageSelectionChanged(uint16_t messageId, bool selected);

    /**
     * @brief Emitted when user toggles signal selection checkbox.
     * @param messageId The message ID containing the signal
     * @param signalName The signal name (unique within the message)
     * @param selected Whether the signal is selected
     */
    void signalSelectionChanged(uint16_t messageId, const QString& signalName, bool selected);

    /**
     * @brief Emitted when user changes a signal value.
     * @param messageId The message ID containing the signal
     * @param signalName The signal name (unique within the message)
     * @param newValue The new signal value
     */
    void signalValueChanged(uint16_t messageId, const QString& signalName, double newValue);

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle() const;

    QScrollArea* m_outerScrollArea;
    Core::CardWidget* m_messagesCard;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_cardsLayout;
    QLabel* m_noDbcLabel;

    RepeatedSendingCard* m_repeatedSendingCard;
    QPushButton* m_sendButton;
};

}  // namespace Sending