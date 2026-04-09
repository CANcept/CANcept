#pragma once
#include <QTableView>
#include <QWidget>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_switch.hpp"
#include "fault_injector/service/fault_handler.hpp"
#include "fault_injector/ui/model/fault_injector_model.hpp"

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

    FaultInjectorModel* m_model;

    Core::CardWidget* m_card = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    Core::StyledSwitch* m_toggleSwitch = nullptr;
    Core::CardWidget* m_tableCardWidget = nullptr;
    QTableView* m_faults = nullptr;
};

}  // namespace FaultInjector