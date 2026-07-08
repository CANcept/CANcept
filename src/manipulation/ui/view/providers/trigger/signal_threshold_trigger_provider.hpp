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
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWidget>

#include "core/widgets/common/styled_combo_box.hpp"
#include "manipulation/constants.hpp"
#include "manipulation/ui/view/providers/dbc_signal_combo.hpp"
#include "manipulation/ui/view/providers/i_manipulation_row_type_provider.hpp"

namespace Manipulation {

class SignalThresholdTriggerProvider final : public IManipulationRowTypeProvider
{
   public:
    explicit SignalThresholdTriggerProvider(const Core::DbcConfig* dbcConfig)
        : m_dbcConfig(dbcConfig)
    {
    }

    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Signal Threshold";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* nameCombo = new Core::StyledComboBox(container);
        nameCombo->setObjectName(Constants::PARAM_SIGNAL_NAME_INPUT);
        populateSignalNameCombo(nameCombo, m_dbcConfig);

        auto* comparison = new Core::StyledComboBox(container);
        comparison->setObjectName(Constants::PARAM_IS_GREATER_INPUT);
        comparison->addItem(">");
        comparison->addItem("<");

        auto* thresholdSpin = new QDoubleSpinBox(container);
        thresholdSpin->setObjectName(Constants::PARAM_THRESHOLD_INPUT);
        thresholdSpin->setRange(-1e9, 1e9);
        thresholdSpin->setDecimals(3);
        thresholdSpin->setSingleStep(1.0);
        thresholdSpin->setValue(0.0);

        layout->addWidget(nameCombo, 1);
        layout->addWidget(comparison);
        layout->addWidget(thresholdSpin);
        return container;
    }

   private:
    const Core::DbcConfig* m_dbcConfig;
};

}  // namespace Manipulation