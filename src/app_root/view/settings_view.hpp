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

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "app_root/model/settings_model.hpp"
#include "core/dto/setting_dto.hpp"

namespace AppRoot {

/** @brief Rendering trait dispatched per SettingType. Specialize to add new types. */
template <Core::SettingType>
struct SettingRenderer;

/** @brief Renders a select-type setting as a card with StyledComboBox. */
template <>
struct SettingRenderer<Core::SettingType::Select> {
    /** @brief Creates the select setting widget. */
    static auto create(const Core::ISetting* setting, SettingsModel* model,
                       QWidget* parent) -> QWidget*;
};

/** @brief The defintion of the Presentation of the settings. */
class SettingsView final : public QWidget
{
    Q_OBJECT

   public:
    /** @brief Constructs the settings view with the given model. */
    explicit SettingsView(SettingsModel* model, QWidget* parent = nullptr);
    ~SettingsView() override = default;

    /** @brief Rebuilds the entire settings UI from the current registry state. */
    void rebuild() const;

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi();
    void buildComponentSection(const std::string& componentId) const;
    void applyStyle() const;

    /** @brief Dispatches to the correct SettingRenderer based on ISetting::getType(). */
    static auto createSettingWidget(const Core::ISetting* setting, SettingsModel* model,
                                    QWidget* parent) -> QWidget*;

    SettingsModel* m_model;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QLabel* m_copyrightLabel;
};

}  // namespace AppRoot
