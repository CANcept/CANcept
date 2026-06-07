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
#include <QPushButton>
#include <QWidget>
#include <memory>
#include <string>
#include <vector>

#include "core/widgets/common/styled_combo_box.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/view/providers/variable_type_provider.hpp"

namespace Math {
/**
 * @brief A single row in the variable configuration dialog.
 *
 * Layout: [Symbol input] [Type dropdown] [Type-specific options] [Delete button]
 */
class VariableRowWidget final : public QWidget
{
    Q_OBJECT

   public:
    using Providers = std::vector<std::unique_ptr<IVariableTypeProvider>>;

    explicit VariableRowWidget(const Providers& providers, QWidget* parent = nullptr);

    void setFromBinding(const VariableBinding& binding);

    [[nodiscard]] auto symbol() const -> char;
    [[nodiscard]] auto configKey() const -> std::string;
    [[nodiscard]] auto displayName() const -> std::string;
    [[nodiscard]] auto typeIndex() const -> int;
    [[nodiscard]] auto createVariable() const -> std::unique_ptr<IVariable>;
    [[nodiscard]] auto isValid() const -> bool;

   signals:
    void removeRequested();

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void onTypeChanged(int index);
    void applyStyle();

    const Providers& m_providers;
    Core::StyledLineEdit* m_symbolInput;
    Core::StyledComboBox* m_typeDropdown;
    QHBoxLayout* m_optionsLayout;
    QWidget* m_currentOptions = nullptr;
    QPushButton* m_deleteButton;
};

}  // namespace Math
