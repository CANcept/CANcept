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
#include <QWidget>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/common/styled_switch.hpp"

namespace Sending {

/**
 * @class RepeatedSendingCard
 * @brief Card widget for configuring repeated/cyclic CAN message transmission.
 *
 * This card provides a toggle to enable repeated sending and an input field
 * for configuring the transmission frequency/interval.
 */
class RepeatedSendingCard final : public QWidget
{
    Q_OBJECT

   public:
    explicit RepeatedSendingCard(QWidget* parent = nullptr);
    ~RepeatedSendingCard() override = default;

    /**
     * @brief Returns whether repeated sending is enabled.
     * @return true if the toggle is checked, false otherwise.
     */
    [[nodiscard]] auto isRepeatedSendingEnabled() const -> bool;

    /**
     * @brief Returns the frequency input field.
     * @return Pointer to the frequency line edit widget.
     */
    [[nodiscard]] auto frequencyEditor() const -> Core::StyledLineEdit*
    {
        return m_frequencyEditor;
    }

   signals:
    /**
     * @brief Emitted when the repeated sending toggle changes state.
     * @param enabled true if repeated sending is enabled, false otherwise
     */
    void toggled(bool enabled);

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void applyStyle() const;
    void onToggleChanged(bool checked);

    Core::CardWidget* m_card;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    Core::StyledSwitch* m_toggleSwitch;
    Core::StyledLineEdit* m_frequencyEditor;
    QLabel* m_frequencyLabel;
};

}  // namespace Sending
