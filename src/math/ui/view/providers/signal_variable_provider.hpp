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

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include "core/dto/dbc_dto.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "variable_type_provider.hpp"

namespace Math {

/**
 * @brief Provider for CAN signal variables with message/signal dropdown selection.
 */
class SignalVariableProvider final : public IVariableTypeProvider
{
   public:
    explicit SignalVariableProvider(const Core::DbcConfig* dbcConfig) : m_dbcConfig(dbcConfig) {}

    auto typeName() const -> QString override
    {
        return QStringLiteral("Signal");
    }

    auto createOptionsWidget(QWidget* parent) -> QWidget* override;

    auto configKey(const QWidget* optionsWidget) const -> std::string override;
    void restoreFromVariable(QWidget* optionsWidget, const IVariable* variable) const override;
    auto displayName(const QWidget* optionsWidget) const -> std::string override;
    auto createVariable(const QWidget* optionsWidget) const -> std::unique_ptr<IVariable> override;

   private:
    static auto findCombos(const QWidget* optionsWidget)
        -> std::pair<const QComboBox*, const QComboBox*>;

    const Core::DbcConfig* m_dbcConfig;
};

}  // namespace Math
