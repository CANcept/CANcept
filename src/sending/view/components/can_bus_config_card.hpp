#pragma once

#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"

namespace Sending {

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
     * @param showInterface Whether to show the interface selection card
     * @param parent Parent widget
     */
    explicit CanBusConfigCard(bool showInterface = true, QWidget* parent = nullptr);
    ~CanBusConfigCard() override = default;

    /**
     * @brief Returns the interface selector combo box.
     * @return Pointer to the interface combo box (may be nullptr if showInterface=false)
     */
    [[nodiscard]] auto interfaceSelector() const -> Core::StyledComboBox*
    {
        return m_interfaceCombo;
    }

    /**
     * @brief Returns the baud rate selector combo box.
     * @return Pointer to the baud rate combo box (may be nullptr if showBaudRate=false)
     */
    [[nodiscard]] auto baudRateSelector() const -> Core::StyledComboBox*
    {
        return m_baudRateCombo;
    }

    /**
     * @brief Populates the interface dropdown with available interfaces.
     * @param interfaces List of available CAN interface names
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   private:
    void setupUi(bool showInterface);

    Core::CardWidget* m_configCard;
    Core::CardWidget* m_interfaceCard;
    Core::CardWidget* m_baudRateCard;
    Core::StyledComboBox* m_interfaceCombo;
    Core::StyledComboBox* m_baudRateCombo;
};

}  // namespace Sending
