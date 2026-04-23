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
#include <QSpinBox>

#include "fault_injector/constants.hpp"
#include "fault_injector/ui/view/providers/i_fault_row_type_provider.hpp"

namespace FaultInjector {

class DlcTriggerProvider final : public IFaultRowTypeProvider
{
   public:
    [[nodiscard]] auto typeName() const -> QString override
    {
        return "DLC";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* spin = new QSpinBox(parent);
        spin->setObjectName(Constants::PARAM_DLC_INPUT);
        spin->setRange(0, 8);
        return spin;
    }
};

}  // namespace FaultInjector