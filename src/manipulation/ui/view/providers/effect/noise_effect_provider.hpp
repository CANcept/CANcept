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
#include <QLineEdit>
#include <QWidget>

#include "manipulation/constants.hpp"
#include "manipulation/ui/view/providers/i_manipulation_row_type_provider.hpp"

namespace Manipulation {

class NoiseEffectProvider final : public IManipulationRowTypeProvider
{
   public:
    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Noise";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* nameEdit = new QLineEdit(container);
        nameEdit->setObjectName(Constants::PARAM_SIGNAL_NAME_INPUT);
        nameEdit->setPlaceholderText("signal name");

        auto* ampSpin = new QDoubleSpinBox(container);
        ampSpin->setObjectName(Constants::PARAM_AMPLITUDE_INPUT);
        ampSpin->setRange(0.0, 1e9);
        ampSpin->setDecimals(3);
        ampSpin->setPrefix("amp ");
        ampSpin->setValue(1.0);

        layout->addWidget(nameEdit, 1);
        layout->addWidget(ampSpin);
        return container;
    }
};

}  // namespace Manipulation