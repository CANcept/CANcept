#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "components/can_bus_config_card.hpp"
#include "components/send_message_button.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/styled_combo_box.hpp"

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
 *
 * @note The Message Data field is a single input where users enter space-separated
 * hex bytes (e.g., "01 02 03 04"). DLC is calculated dynamically from the input.
 *
 * @note This class is a "Passive View." It exposes widgets via accessors
 * so the @ref SendingDelegate can map them to the @ref SendingModel.
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
     * @brief Populates the interface dropdown (Called by Delegate).
     * @param interfaces List of available CAN interface names (e.g., "can0", "vcan0").
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces);

    /**
     * @brief Populates the baud rate dropdown (Called by Delegate).
     * @param baudRates List of available baud rates (e.g., 125000, 250000, 500000).
     */
    void setAvailableBaudRates(const std::vector<uint32_t>& baudRates);

   private:
    void setupUi();

    // CAN-Bus Configuration Card
    CanBusConfigCard* m_configCard;

    // CAN Frame Card
    Core::CardWidget* m_frameCard;
    QLineEdit* m_canIdEditor;
    QLineEdit* m_messageDataEditor;

    // Floating Send Button
    QPushButton* m_sendButton;
};

}  // namespace Sending