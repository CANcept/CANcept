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
#include <QLineEdit>

#include "manipulation/constants.hpp"
#include "manipulation/ui/view/providers/i_manipulation_row_type_provider.hpp"

namespace Manipulation {

class SignalNameTriggerProvider final : public IManipulationRowTypeProvider
{
   public:
    [[nodiscard]] auto typeName() const -> QString override
    {
        return "Signal Name";
    }

    [[nodiscard]] auto createOptionsWidget(QWidget* parent) const -> QWidget* override
    {
        auto* edit = new QLineEdit(parent);
        edit->setObjectName(Constants::PARAM_SIGNAL_NAME_INPUT);
        edit->setPlaceholderText("signal name");
        return edit;
    }
};

}  // namespace Manipulation