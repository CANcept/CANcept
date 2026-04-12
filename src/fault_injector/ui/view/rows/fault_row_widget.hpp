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
#include <vector>

#include "core/widgets/common/styled_combo_box.hpp"
#include "fault_injector/ui/model/section_entry.hpp"
#include "fault_injector/ui/view/providers/i_fault_row_type_provider.hpp"

namespace FaultInjector {

/**
 * @class FaultRowWidget
 * @brief A single configurable row inside a fault section (trigger, effect and strategy).
 *
 * Layout: [Type combo] [Type-specific options...] [X]
 */
class FaultRowWidget final : public QWidget
{
    Q_OBJECT

   public:
    using Providers = std::vector<std::unique_ptr<IFaultRowTypeProvider>>;

    /**
     * @brief Constructs the row.
     * @param providers Type providers defining available choices. Must not be empty.
     * @param showRemove Whether to show the X remove button.
     * @param parent Parent widget.
     */
    explicit FaultRowWidget(Providers providers, bool showRemove = true, QWidget* parent = nullptr);

    /**
     * @brief Returns a SectionEntry reflecting the current combo index and input values.
     */
    [[nodiscard]] auto currentEntry() const -> SectionEntry;

    /**
     * @brief Pre-populates the row from a SectionEntry without emitting changed().
     */
    void setFromEntry(const SectionEntry& entry);

   signals:
    void removeRequested();
    /**
     * @brief Emitted whenever the selected type or any parameter value changes.
     */
    void changed();

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void onTypeChanged(int index);
    void connectOptionsSignals() const;
    void applyStyle() const;

    Providers m_providers;
    Core::StyledComboBox* m_typeCombo;
    QHBoxLayout* m_optionsLayout;
    QWidget* m_currentOptions = nullptr;
    QPushButton* m_removeButton;
};

}  // namespace FaultInjector