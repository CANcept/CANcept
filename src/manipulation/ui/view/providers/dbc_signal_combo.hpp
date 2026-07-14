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
#include <set>
#include <string>

#include "core/dto/dbc_dto.hpp"
#include "core/widgets/common/styled_combo_box.hpp"

namespace Manipulation {

/**
 * @brief Populates a combo box with every unique signal name across the whole DBC, sorted.
 *
 * A DbcManipulation's triggers/effects aren't scoped to a single message, so signal
 * names are offered flat and deduplicated rather than grouped by message.
 *
 * @param combo the combo box to populate
 * @param dbcConfig the currently loaded DBC config, or nullptr if none is loaded
 */
inline void populateSignalNameCombo(Core::StyledComboBox* combo, const Core::DbcConfig* dbcConfig)
{
    std::set<std::string> names;
    if (dbcConfig)
    {
        for (const auto& msg : dbcConfig->messageDefinitions)
        {
            for (const auto& sig : msg.signalDescriptions)
            {
                names.insert(sig.signalName);
            }
        }
    }
    for (const auto& name : names)
    {
        combo->addItem(QString::fromStdString(name));
    }
}

}  // namespace Manipulation
