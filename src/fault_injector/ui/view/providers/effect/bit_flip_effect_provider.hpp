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
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QWidget>

#include "fault_injector/constants.hpp"
#include "fault_injector/ui/view/providers/i_fault_row_type_provider.hpp"

namespace FaultInjector {

class BitFlipEffectProvider final : public IFaultRowTypeProvider
{
   public:
    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Bit Flip";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* byteLabel = new QLabel("byte:", container);
        auto* byteSpin = new QSpinBox(container);
        byteSpin->setObjectName(Constants::PARAM_BYTE_INPUT);
        byteSpin->setRange(0, 7);

        auto* bitLabel = new QLabel("bit:", container);
        auto* bitSpin = new QSpinBox(container);
        bitSpin->setObjectName(Constants::PARAM_BIT_INPUT);
        bitSpin->setRange(0, 7);

        layout->addWidget(byteLabel);
        layout->addWidget(byteSpin);
        layout->addWidget(bitLabel);
        layout->addWidget(bitSpin);
        layout->addStretch();
        return container;
    }
};

}  // namespace FaultInjector