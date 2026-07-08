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

#include "manipulation/constants.hpp"
#include "manipulation/ui/view/providers/i_manipulation_row_type_provider.hpp"

namespace Manipulation {

/**
 * @brief Strategy provider for inserting a brand-new DBC message after a delay.
 *
 * Only the delay input lives here, right next to the strategy combo (mirroring
 * DelayedStrategyProvider) - the message picker and per-signal value editor live in a
 * separate conditionally-shown area managed by ManipulationDialog, since they need a
 * dynamically rebuilt multi-row card rather than a single inline widget.
 */
class InsertStrategyProvider final : public IManipulationRowTypeProvider
{
   public:
    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Insert";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* label = new QLabel("delay (µs):", container);
        auto* spin = new QSpinBox(container);
        spin->setObjectName(Constants::PARAM_DELAY_INPUT);
        spin->setRange(0, 10'000'000);
        spin->setValue(1000);

        layout->addWidget(label);
        layout->addWidget(spin);
        layout->addStretch();
        return container;
    }
};

}  // namespace Manipulation
