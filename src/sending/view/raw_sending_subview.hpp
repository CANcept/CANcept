#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "components/can_bus_config_card.hpp"
#include "components/hex_id_line_edit.hpp"
#include "components/send_message_button.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_combo_box.hpp"
#include "sending/view/formatter/hex_data_formatter.hpp"

namespace Sending {

/**
 * @class RawSendingSubView
 * @brief Modern card-based view for raw CAN frame composition.
 *
 * @section Layout Architecture
 * The view uses a card-based design matching the Figma mockups:
 * 1. CAN-Bus Configuration Card: Interface and Baud Rate selection
 * 2. CAN Frame Card: Single-line hex inputs for ID and Message Data
 * 3. Repeat Settings Card: (Optional - not yet implemented)
 * 4. Floating Send Message button at bottom right
 */
class RawSendingSubView final : public QWidget
{
    Q_OBJECT

   public:
    explicit RawSendingSubView(QWidget* parent = nullptr);
    ~RawSendingSubView() override = default;

    /**
     * @name Configuration Accessors
     * @{
     */
    [[nodiscard]] auto interfaceSelector() const -> QComboBox*
    {
        return m_configCard ? m_configCard->interfaceSelector() : nullptr;
    }
    [[nodiscard]] auto baudRateSelector() const -> QComboBox*
    {
        return m_configCard ? m_configCard->baudRateSelector() : nullptr;
    }
    /** @} */

    /**
     * @name Frame Data Accessors
     * @{
     */
    [[nodiscard]] auto canIdEditor() const -> QLineEdit*
    {
        return m_canIdEditor;
    }
    [[nodiscard]] auto messageDataEditor() const -> QLineEdit*
    {
        return m_messageDataEditor;
    }
    /** @} */

    /**
     * @name Control Accessors
     * @{
     */
    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }
    /** @} */

    /**
     * @brief Populates the interface dropdown.
     * @param interfaces List of available CAN interface names (e.g., "can0", "vcan0").
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   private:
    void setupUi();
    void setupCanIdInput() const;
    void setupMessageDataInput();

    // CAN-Bus Configuration Card
    CanBusConfigCard* m_configCard;

    // CAN Frame Card
    Core::CardWidget* m_frameCard;
    HexIdLineEdit* m_canIdEditor;
    QLineEdit* m_messageDataEditor;
    HexDataFormatter* m_messageDataFormatter;

    // Floating Send Button
    QPushButton* m_sendButton;
};

}  // namespace Sending