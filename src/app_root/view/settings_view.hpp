#pragma once

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
    void applyStyle();
    void buildComponentSection(const std::string& componentId) const;

    /** @brief Dispatches to the correct SettingRenderer based on ISetting::getType(). */
    static auto createSettingWidget(Core::ISetting* setting, SettingsModel* model,
                                    QWidget* parent) -> QWidget*;

    SettingsModel* m_model;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
};

}  // namespace AppRoot
