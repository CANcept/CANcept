#include "can_bus_config_card.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

CanBusConfigCard::CanBusConfigCard(const bool showInterface, const bool showBaudRate,
                                   QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCard(nullptr),
      m_baudRateCard(nullptr),
      m_interfaceCombo(nullptr),
      m_baudRateCombo(nullptr)
{
    setupUi(showInterface, showBaudRate);
}

void CanBusConfigCard::setupUi(const bool showInterface, const bool showBaudRate)
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    m_configCard = new Core::CardWidget(Constants::CAN_CONFIGURATION_TITLE, QString(),
                                        Constants::CONFIGURATION_ICON_PATH, this);
    auto* innerCardsRow = new QHBoxLayout();
    innerCardsRow->setSpacing(spacing.spacingLg);

    // Interface Card
    if (showInterface)
    {
        m_interfaceCard =
            new Core::CardWidget(QString(), Constants::INTERFACE_LABEL, QString(), m_configCard);
        m_interfaceCombo = new Core::StyledComboBox(m_interfaceCard);
        m_interfaceCombo->setPlaceholderText(Constants::INTERFACE_PLACEHOLDER);
        m_interfaceCard->contentLayout()->addWidget(m_interfaceCombo);
        innerCardsRow->addWidget(m_interfaceCard);
    }

    // Baud Rate Card
    if (showBaudRate)
    {
        m_baudRateCard =
            new Core::CardWidget(QString(), Constants::BAUD_RATE_LABEL, QString(), m_configCard);
        m_baudRateCombo = new Core::StyledComboBox(m_baudRateCard);
        m_baudRateCombo->setPlaceholderText(Constants::BAUD_RATE_PLACEHOLDER);
        m_baudRateCard->contentLayout()->addWidget(m_baudRateCombo);
        innerCardsRow->addWidget(m_baudRateCard);
    }

    m_configCard->contentLayout()->addLayout(innerCardsRow);
    mainLayout->addWidget(m_configCard);
}

void CanBusConfigCard::setAvailableInterfaces(const std::vector<std::string>& interfaces) const
{
    if (!m_interfaceCombo)
    {
        return;
    }

    m_interfaceCombo->clear();
    for (const auto& interface : interfaces)
    {
        m_interfaceCombo->addItem(QString::fromStdString(interface));
    }
}

void CanBusConfigCard::setAvailableBaudRates(const std::vector<uint32_t>& baudRates) const
{
    if (!m_baudRateCombo)
    {
        return;
    }

    m_baudRateCombo->clear();
    for (const auto& rate : baudRates)
    {
        m_baudRateCombo->addItem(QString::number(rate), QVariant(rate));
    }
}

}  // namespace Sending
