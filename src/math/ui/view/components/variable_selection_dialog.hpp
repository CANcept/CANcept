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

#include <QDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <memory>
#include <string>
#include <vector>

#include "../providers/variable_type_provider.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "variable_row_widget.hpp"

namespace Math {

/**
 * @brief Dialog for configuring expression variables.
 *
 * Each row lets the user assign a symbol, pick a variable type, and configure
 * type-specific options.
 */
class VariableSelectionDialog final : public QDialog
{
    Q_OBJECT

   public:
    using Providers = std::vector<std::unique_ptr<IVariableTypeProvider>>;

    explicit VariableSelectionDialog(Providers providers,
                                     const std::vector<VariableBinding>& currentBindings,
                                     QWidget* parent = nullptr);

    struct RowResult {
        VariableBinding binding;
        std::string configKey;
        std::size_t rowIndex;
    };

    /**
     * @brief Returns the valid, deduplicated rows.
     *
     * Duplicate symbols are dropped (first occurrence wins).
     */
    [[nodiscard]] auto resultBindings() const -> std::vector<RowResult>;

    /**
     * @brief Creates a variable for the row at the given index into m_rows.
     */
    [[nodiscard]] auto createVariableForRow(std::size_t rowIndex) const
        -> std::unique_ptr<IVariable>;

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi(const std::vector<VariableBinding>& currentBindings);
    void addRow(const VariableBinding* binding = nullptr);
    void removeRow(VariableRowWidget* row);
    void applyStyle();

    Providers m_providers;

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollContent = nullptr;
    QVBoxLayout* m_scrollLayout = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_confirmButton = nullptr;

    std::vector<VariableRowWidget*> m_rows;
};

}  // namespace Math
