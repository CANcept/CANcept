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
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <memory>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_switch.hpp"
#include "fault_injector/service/fault_handler.hpp"
#include "fault_injector/ui/model/fault_injector_model.hpp"
#include "fault_injector/ui/view/fault_injector_dialog.hpp"

namespace FaultInjector {

class FaultInjectorView final : public QWidget
{
    Q_OBJECT

   public:
    explicit FaultInjectorView(QWidget* parent = nullptr);
    ~FaultInjectorView() override = default;

    /**
     * @brief Returns whether fault injection is enabled.
     * @return true if the toggle is checked, false otherwise.
     */
    [[nodiscard]] auto isFaultInjection() const -> bool;
    /**
     * @brief Returns the fault handler.
     * @return the current fault handler.
     */
    [[nodiscard]] auto getFaultHandler() const -> FaultHandler;

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi();
    void applyStyle() const;
    void onToggleChanged(bool checked);

    void onAddRawClicked() const;
    void onAddDbcClicked() const;
    void onFaultClicked(const QModelIndex& index) const;

    FaultInjectorModel* m_model;
    FaultInjectorDialog* m_dialog = nullptr;

    Core::CardWidget* m_card = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    Core::StyledSwitch* m_toggleSwitch = nullptr;
    QPushButton* m_addRawButton = nullptr;
    QPushButton* m_addDbcButton = nullptr;
    Core::CardWidget* m_tableCardWidget = nullptr;
    QTableView* m_faults = nullptr;
};

}  // namespace FaultInjector