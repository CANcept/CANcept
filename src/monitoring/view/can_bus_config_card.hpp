#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

#include "core/widgets/card_widget.hpp"
#include "core/widgets/styled_combo_box.hpp"

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

    /**
     * @brief Populates the interface dropdown with available interfaces.
     * @param interfaces List of available CAN interface names
     */
    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   private:
    void setupUi();

    // Helper to create the styled stat boxes
    auto createStatBox(const QString& title, QLabel*& valueLabel) -> QFrame*;

    Core::CardWidget* m_configCard;
    Core::CardWidget* m_interfaceCard;
    Core::CardWidget* m_statusCard;
    Core::CardWidget* m_frameRateCard;
    Core::CardWidget* m_messageCountCard;

    Core::StyledComboBox* m_interfaceCombo;
    QPushButton* m_connectionButton;

    // Row 1
    QLabel* m_titleIcon;
    QCheckBox* m_dbcCheck;
    QComboBox* m_combobox;
    QPushButton* m_connectButton;

    // Row 2 Content Labels
    QLabel* m_fpsValueLabel;
    QLabel* m_statusValueLabel;
    QLabel* m_msgCountValueLabel;
};

}  // namespace Monitoring
