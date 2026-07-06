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
#include "manipulation/service/manipulation_handler.hpp"
#include "manipulation/ui/model/manipulation_model.hpp"
#include "manipulation/ui/view/manipulation_dialog.hpp"

namespace Manipulation {

class ManipulationView final : public QWidget
{
    Q_OBJECT

   public:
    explicit ManipulationView(QWidget* parent = nullptr);
    ~ManipulationView() override = default;

    /**
     * @brief Returns whether manipulation is enabled.
     * @return true if the toggle is checked, false otherwise.
     */
    [[nodiscard]] auto isManipulation() const -> bool;

    /**
     * @brief Returns the manipulation handler.
     * @return the current manipulation handler.
     */
    [[nodiscard]] auto getManipulationHandler() const -> ManipulationHandler;

    /**
     * @brief Switches between Raw and Dbc mode.
     *
     * In Raw mode the DBC button is hidden and any existing DbcManipulations are removed.
     * In Dbc mode both add-buttons are available.
     */
    void setMode(ManipulationModel::Mode mode);

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUi();
    void applyStyle() const;
    void onToggleChanged(bool checked);

    void onAddRawClicked() const;
    void onAddDbcClicked() const;
    void onManipulationClicked(const QModelIndex& index) const;

    ManipulationModel* m_model;
    ManipulationDialog* m_dialog = nullptr;

    Core::CardWidget* m_card = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    Core::StyledSwitch* m_toggleSwitch = nullptr;
    QPushButton* m_addRawButton = nullptr;
    QPushButton* m_addDbcButton = nullptr;
    Core::CardWidget* m_tableCardWidget = nullptr;
    QTableView* m_manipulations = nullptr;
};

}  // namespace Manipulation