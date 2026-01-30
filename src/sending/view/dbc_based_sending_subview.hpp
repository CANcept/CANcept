#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include <string>
#include <vector>

#include "components/can_bus_config_card.hpp"
#include "components/send_message_button.hpp"
#include "core/widgets/dbc_message_card.hpp"

namespace Core {
class CardWidget;
}

namespace Sending {

class SendingModel;

/**
 * @class DbcSendingSubView
 * @brief The primary container for the DBC-based workflow.
 */
class DbcSendingSubView final : public QWidget
{
    Q_OBJECT
   public:
    explicit DbcSendingSubView(QWidget* parent = nullptr);
    ~DbcSendingSubView() override = default;

    /**
     * @brief Populates the view by reading from the model.
     * Creates message cards and signal rows based on model data.
     * This maintains MVD separation - View reads Model, doesn't modify it.
     */
    void populateFromModel(const SendingModel* model);

    /**
     * @brief Clears all message cards.
     */
    void clearMessages() const;

    /**
     * @name Control Accessors
     */
    [[nodiscard]] auto interfaceSelector() const -> QComboBox*
    {
        return m_configCard ? m_configCard->interfaceSelector() : nullptr;
    }
    [[nodiscard]] auto sendButton() const -> QPushButton*
    {
        return m_sendButton;
    }

    void setAvailableInterfaces(const std::vector<std::string>& interfaces) const;

   signals:
    /**
     * @brief Emitted when user toggles message selection checkbox.
     */
    void messageSelectionChanged(uint32_t messageId, bool selected);

    /**
     * @brief Emitted when user toggles signal selection checkbox.
     */
    void signalSelectionChanged(const QString& signalName, bool selected);

    /**
     * @brief Emitted when user changes a signal value.
     */
    void signalValueChanged(const QString& signalName, double newValue);

   private:
    void setupUi();

    CanBusConfigCard* m_configCard;

    Core::CardWidget* m_messagesCard;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_cardsLayout;

    QPushButton* m_sendButton;
};

}  // namespace Sending