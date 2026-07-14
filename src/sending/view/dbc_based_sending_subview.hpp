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
#include <memory>

#include "components/repeated_sending_card.hpp"
#include "components/send_message_button.hpp"
#include "core/interface/i_manipulation_handler.hpp"
#include "core/service/serializer.hpp"
#include "core/widgets/common/link_button.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "manipulation/service/manipulation_handler.hpp"
#include "manipulation/ui/view/manipulation_view.hpp"

namespace Math {
class VariableRegistry;
}

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
     * @brief Populates the view with message cards and signal rows from the model.
     */
    void populateFromModel(SendingModel* model, Math::VariableRegistry& registry);

    /**
     * @brief Clears all message cards.
     */
    void clearMessages() const;

    /**
     * @brief Tells the manipulation view no DBC config is available anymore.
     */
    void clearManipulationDbcConfig() const;

    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }

    [[nodiscard]] auto repeatedSendingCard() const -> RepeatedSendingCard*
    {
        return m_repeatedSendingCard;
    }

    /**
     * @brief Returns a manipulation handler snapshot if injection is enabled, nullptr otherwise.
     */
    [[nodiscard]] auto getManipulationHandler() const -> std::shared_ptr<Core::IManipulationHandler>
    {
        if (m_manipulation && m_manipulation->isManipulation())
        {
            return std::make_shared<Manipulation::ManipulationHandler>(
                m_manipulation->getManipulationHandler());
        }
        return nullptr;
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

    /**
     * @brief Builds a serializer covering the full DBC sending configuration: cyclic settings,
     * manipulations, and every signal row's value function. Used identically for both save and
     * load.
     */
    [[nodiscard]] auto buildStateSerializer() -> Core::Serializer;
    void onSaveClicked();
    void onLoadClicked();

    QScrollArea* m_outerScrollArea;
    Core::CardWidget* m_messagesCard;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_cardsLayout;
    QLabel* m_noDbcLabel;

    RepeatedSendingCard* m_repeatedSendingCard;
    Manipulation::ManipulationView* m_manipulation;
    QPushButton* m_sendButton;

    // Configuration save/load
    Core::LinkButton* m_loadButton;
    Core::LinkButton* m_saveButton;

    /** @brief Stored from the last populateFromModel() call, needed to re-acquire variables
     * when loading value functions. */
    Math::VariableRegistry* m_registry = nullptr;
};

}  // namespace Sending