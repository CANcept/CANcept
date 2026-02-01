#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"

namespace Monitoring {
/**
 * @class CanBusConfigCard
 * @brief Reusable CAN-Bus configuration card with optional interface and baud rate selection.
 *
 * This component encapsulates the CAN-Bus Configuration card with nested cards for
 * interface and baud rate selection.
 */
class CanBusConfigCard final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the CAN-Bus Configuration card.
     * @param parent Parent widget
     */
    explicit CanBusConfigCard(QWidget* parent = nullptr);
    ~CanBusConfigCard() override = default;

    /**
     * @brief Returns the interface selector combo box.
     * @return Pointer to the interface combo box (may be nullptr if showInterface=false)
     */
    [[nodiscard]] auto interfaceSelector() const -> Core::StyledComboBox*
    {
        return m_interfaceCombo;
    }

    [[nodiscard]] auto modeToggle() const -> QPushButton*
    {
        return m_dbcToggleButton;
    }

    /**
     * @brief Populates the interface dropdown with available interfaces.
     * @param interfaces List of available CAN interface names
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   private:
    void setupUi();

    Core::CardWidget* m_configCard;
    QLabel* m_titleIcon;
    Core::CardWidget* m_interfaceCard;
    Core::StyledComboBox* m_interfaceCombo;

    Core::CardWidget* m_statusCard;
    QPushButton* m_dbcToggleButton;
    QLabel* m_statusValueLabel;

    Core::CardWidget* m_frameRateCard;
    QLabel* m_fpsValueLabel;

    Core::CardWidget* m_messageCountCard;
    QLabel* m_msgCountValueLabel;
};

}  // namespace Monitoring
