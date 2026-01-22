#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <vector>

namespace Sending {

/**
 * @class RawSendingSubView
 * @brief The specialized view for bit-level CAN frame composition.
 * * @section Layout Architecture
 * The view is organized into three vertical logical groups based on the transmission workflow:
 * 1. Header (Frame Config): Arbitration ID, Extended Flag, and DLC.
 * 2. Body (Payload): A visual grid of 8 byte editors.
 * 3. Footer (Control): Physical interface selection and the execution trigger.
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
     * Used by the Delegate to bind ID/DLC to the Model.
     * @{
     */
    [[nodiscard]] auto idEditor() const -> QLineEdit*
    {
        return m_idEditor;
    }
    [[nodiscard]] auto dlcSpinBox() const -> QSpinBox*
    {
        return m_dlcSpin;
    }
    /** @} */

    /**
     * @name Payload Accessors
     * @return A vector of 8 line edits representing Byte 0 - Byte 7.
     */
    [[nodiscard]] auto byteEditors() const -> const std::vector<QLineEdit*>&
    {
        return m_byteEditors;
    }

    /**
     * @name Control Accessors
     * Used by the Delegate to sync hardware availability and trigger sends.
     */
    [[nodiscard]] auto deviceSelector() const -> QComboBox*
    {
        return m_deviceCombo;
    }
    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }

    /**
     * @brief Populates the device dropdown (Called by Delegate).
     * @param devices List of available CAN interface names (e.g., "can0", "vcan0").
     */
    void setAvailableDevices(const std::vector<std::string>& devices);

   private:
    void setupUi();

    QGroupBox* m_configGroup;
    QLineEdit* m_idEditor;
    QSpinBox* m_dlcSpin;

    QGroupBox* m_payloadGroup;
    std::vector<QLineEdit*> m_byteEditors;

    QGroupBox* m_actionGroup;
    QComboBox* m_deviceCombo;
    QPushButton* m_sendButton;
};

}  // namespace Sending