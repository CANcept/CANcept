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

#include "manipulation/constants.hpp"
#include "manipulation/ui/view/providers/dbc_signal_combo.hpp"
#include "manipulation/ui/view/providers/i_manipulation_row_type_provider.hpp"

namespace Manipulation {

class ClampEffectProvider final : public IManipulationRowTypeProvider
{
   public:
    explicit ClampEffectProvider(const Core::DbcConfig* dbcConfig) : m_dbcConfig(dbcConfig) {}

    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Clamp";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* nameCombo = new Core::StyledComboBox(container);
        nameCombo->setObjectName(Constants::PARAM_SIGNAL_NAME_INPUT);
        populateSignalNameCombo(nameCombo, m_dbcConfig);

        auto* minSpin = new QDoubleSpinBox(container);
        minSpin->setObjectName(Constants::PARAM_MIN_VALUE_INPUT);
        minSpin->setRange(-1e9, 1e9);
        minSpin->setDecimals(3);
        minSpin->setPrefix("min ");

        auto* maxSpin = new QDoubleSpinBox(container);
        maxSpin->setObjectName(Constants::PARAM_MAX_VALUE_INPUT);
        maxSpin->setRange(-1e9, 1e9);
        maxSpin->setDecimals(3);
        maxSpin->setPrefix("max ");
        maxSpin->setValue(100.0);

        layout->addWidget(nameCombo, 1);
        layout->addWidget(minSpin);
        layout->addWidget(maxSpin);
        return container;
    }

   private:
    const Core::DbcConfig* m_dbcConfig;
};

}  // namespace Manipulation