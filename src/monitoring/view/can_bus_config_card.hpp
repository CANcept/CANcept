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

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {
/**
 * @class CanBusConfigCard
 * @brief Reusable CAN-Bus configuration card with optional interface and baud rate selection.
 *
 * This component encapsulates the CAN-Bus Configuration card with nested cards for
 * interface and baud rate selection.
 */
class CanBusConfigCard final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the CAN-Bus Configuration card.
     * @param parent Parent widget
     */
    explicit CanBusConfigCard(QWidget* parent = nullptr, MonitoringModel* model = nullptr);
    ~CanBusConfigCard() override = default;

    /**
     * @brief Returns the interface selector combo box.
     * @return Pointer to the interface combo box (may be nullptr if showInterface=false)
     */
    [[nodiscard]] auto interfaceSelector() const -> Core::StyledComboBox*
    {
        return m_interfaceCombo;
    }

    [[nodiscard]] auto modeToggle() const -> QPushButton*
    {
        return m_dbcToggleButton;
    }

    /**
     * @brief Populates the interface dropdown with available interfaces.
     * @param interfaces List of available CAN interface names
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   private:
    void setupUi();

    Core::CardWidget* m_configCard;
    QLabel* m_titleIcon;
    Core::CardWidget* m_interfaceCard;
    Core::StyledComboBox* m_interfaceCombo;

    Core::CardWidget* m_statusCard;
    QPushButton* m_dbcToggleButton;
    QLabel* m_statusValueLabel;

    Core::CardWidget* m_frameRateCard;
    QLabel* m_fpsValueLabel;

    Core::CardWidget* m_messageCountCard;
    QLabel* m_msgCountValueLabel;
    MonitoringModel* m_model;
};

}  // namespace Monitoring
