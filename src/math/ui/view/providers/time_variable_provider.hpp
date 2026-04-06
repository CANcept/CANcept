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

#include "core/widgets/common/styled_combo_box.hpp"
#include "math/service/variables/time_variable.hpp"

namespace Math {

/**
 * @brief Provider for time variables with unit selection.
 */
class TimeVariableProvider final : public IVariableTypeProvider
{
   public:
    auto typeName() const -> QString override
    {
        return QStringLiteral("Time");
    }

    auto createOptionsWidget(QWidget* parent) -> QWidget* override
    {
        auto* combo = new Core::StyledComboBox(parent);
        combo->addItem("Seconds", static_cast<int>(TimeUnit::Seconds));
        combo->addItem("Milliseconds", static_cast<int>(TimeUnit::Milliseconds));
        combo->addItem("Nanoseconds", static_cast<int>(TimeUnit::Nanoseconds));
        return combo;
    }

    void restoreFromVariable(QWidget* optionsWidget, const IVariable* variable) const override
    {
        auto* combo = qobject_cast<QComboBox*>(optionsWidget);
        if (!combo || !variable) return;

        const auto* tv = dynamic_cast<const TimeVariable*>(variable);
        if (!tv) return;

        for (int i = 0; i < combo->count(); ++i)
        {
            if (static_cast<TimeUnit>(combo->itemData(i).toInt()) == tv->unit())
            {
                combo->setCurrentIndex(i);
                return;
            }
        }
    }

    auto configKey(const QWidget* optionsWidget) const -> std::string override
    {
        const auto* combo = qobject_cast<const QComboBox*>(optionsWidget);
        if (!combo) return {};
        const auto unit = static_cast<TimeUnit>(combo->currentData().toInt());
        return "time:" + TimeVariable::unitSuffix(unit);
    }

    auto displayName(const QWidget* optionsWidget) const -> std::string override
    {
        const auto* combo = qobject_cast<const QComboBox*>(optionsWidget);
        if (!combo) return "Time";
        const auto unit = static_cast<TimeUnit>(combo->currentData().toInt());
        switch (unit)
        {
            case TimeUnit::Seconds:
                return "Time (s)";
            case TimeUnit::Milliseconds:
                return "Time (ms)";
            case TimeUnit::Nanoseconds:
                return "Time (ns)";
        }
        return "Time";
    }

    auto createVariable(const QWidget* optionsWidget) const -> std::unique_ptr<IVariable> override
    {
        const auto* combo = qobject_cast<const QComboBox*>(optionsWidget);
        if (!combo) return std::make_unique<TimeVariable>(TimeUnit::Seconds);
        const auto unit = static_cast<TimeUnit>(combo->currentData().toInt());
        return std::make_unique<TimeVariable>(unit);
    }
};

}  // namespace Math
