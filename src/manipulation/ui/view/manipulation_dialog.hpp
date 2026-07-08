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
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "manipulation/types/manipulation.hpp"
#include "manipulation/ui/model/manipulation_dialog_model.hpp"
#include "manipulation/ui/view/rows/manipulation_row_widget.hpp"

namespace Core {
class DbcMessageCard;
class DbcSignalRowWidget;
struct DbcConfig;
}  // namespace Core

namespace Math {
class VariableRegistry;
}  // namespace Math

namespace Manipulation {

/**
 * @brief Dialog for configuring and applying manipulations on CAN bus signals.
 */
class ManipulationDialog : public QDialog
{
    Q_OBJECT
   public:
    explicit ManipulationDialog(QWidget* parent = nullptr);
    ~ManipulationDialog() override = default;

    /**
     * @brief Opens the dialog for a new manipulation in the given mode.
     * @param isRaw true for raw CAN frame editing, false for DBC-based editing.
     */
    auto open(bool isRaw) -> int;

    /**
     * @brief Returns the completed manipulation and resets the dialog model.
     * @return The configured manipulation, or std::nullopt if cancelled or invalid.
     */
    [[nodiscard]] auto acquire() const -> std::optional<ManipulationEntry>;

    /** @brief Sets the DBC-aware registry used to populate signal/message dropdowns. */
    void setVariableRegistry(Math::VariableRegistry* registry);

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void setupUI();
    void applyStyle();

    void onTriggerAdded(int index);
    void onTriggerRemoved(int index);
    void onEffectAdded(int index);
    void onEffectRemoved(int index);
    void onInsertMessageChanged(int index);
    void updateStrategySubAreaVisibility();
    void rebuildStrategyRow(bool isRaw);
    void populateInsertMessageCombo();
    [[nodiscard]] auto dbcConfig() const -> const Core::DbcConfig*;

    std::unique_ptr<ManipulationDialogModel> m_model;
    Math::VariableRegistry* m_variableRegistry = nullptr;
    bool m_dialogHeightLocked = false;

    QVBoxLayout* m_layout = nullptr;

    QLabel* m_titleLabel = nullptr;

    Core::CardWidget* m_triggerCard = nullptr;
    QScrollArea* m_triggerScrollArea = nullptr;
    QVBoxLayout* m_triggerRowsLayout = nullptr;
    QPushButton* m_addTriggerButton = nullptr;
    QList<ManipulationRowWidget*> m_triggerRows;

    Core::CardWidget* m_effectCard = nullptr;
    QScrollArea* m_effectScrollArea = nullptr;
    QVBoxLayout* m_effectRowsLayout = nullptr;
    QPushButton* m_addEffectButton = nullptr;
    QList<ManipulationRowWidget*> m_effectRows;

    QLabel* m_strategyLabel = nullptr;
    ManipulationRowWidget* m_strategyRow = nullptr;

    Core::CardWidget* m_insertCard = nullptr;
    Core::StyledComboBox* m_insertMessageCombo = nullptr;
    QWidget* m_insertMessageContainer = nullptr;
    QVBoxLayout* m_insertMessageContainerLayout = nullptr;
    QScrollArea* m_insertMessageScrollArea = nullptr;
    QLabel* m_insertCurrentMessageHint = nullptr;
    Core::DbcMessageCard* m_insertMessageCard = nullptr;
    std::vector<std::pair<std::string, Core::DbcSignalRowWidget*>> m_insertSignalRows;

    QLabel* m_mutationLabel = nullptr;
    ManipulationRowWidget* m_mutationRow = nullptr;

    QPushButton* m_confirmButton = nullptr;
};

}  // namespace Manipulation