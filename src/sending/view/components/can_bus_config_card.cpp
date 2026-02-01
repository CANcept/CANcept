#include "can_bus_config_card.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

CanBusConfigCard::CanBusConfigCard(const bool showInterface, QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCard(nullptr),
      m_baudRateCard(nullptr),
      m_interfaceCombo(nullptr),
      m_baudRateCombo(nullptr)
{
    setupUi(showInterface);
}

void CanBusConfigCard::setupUi(const bool showInterface)
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    m_configCard = new Core::CardWidget(Constants::CAN_CONFIGURATION_TITLE, QString(),
                                        Constants::CONFIGURATION_ICON_PATH, this);
    auto* innerCardsRow = new QHBoxLayout(this);
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

}  // namespace Sending
