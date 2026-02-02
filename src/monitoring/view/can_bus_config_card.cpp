#include "can_bus_config_card.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

CanBusConfigCard::CanBusConfigCard(QWidget* parent, MonitoringModel* model)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCard(nullptr),
      m_frameRateCard(nullptr),
      m_messageCountCard(nullptr),
      m_interfaceCombo(nullptr),
      m_model(model)
{
    setupUi();
}

void CanBusConfigCard::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    m_configCard = new Core::CardWidget(Constants::CAN_CONFIGURATION_TITLE, QString(),
                                        Constants::CAN_CONFIGURATION_ICON_PATH, this);

    auto* groupLayout = new QHBoxLayout();
    groupLayout->setSpacing(spacing.spacingLg);

    m_interfaceCard =
        new Core::CardWidget(QString(), Constants::INTERFACE_LABEL, QString(), m_configCard);
    m_interfaceCombo = new Core::StyledComboBox(m_interfaceCard);
    m_interfaceCombo->setPlaceholderText(Constants::INTERFACE_PLACEHOLDER);
    m_interfaceCombo->addItems(Constants::DEFAULT_CAN_DEVICES);
    m_interfaceCard->contentLayout()->addWidget(m_interfaceCombo);
    groupLayout->addWidget(m_interfaceCard);

    m_statusCard =
        new Core::CardWidget(QString(), Constants::CAN_STATUS_LABEL, QString(), m_configCard);
    auto* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(spacing.spacingSm);
    m_fpsValueLabel = new QLabel(m_statusCard);
    m_fpsValueLabel->setText(QString("Frames/s: "));
    statusLayout->addWidget(m_fpsValueLabel);
    m_msgCountValueLabel = new QLabel(m_statusCard);
    m_msgCountValueLabel->setText(QString("Message types: "));
    statusLayout->addWidget(m_msgCountValueLabel);
    m_statusCard->contentLayout()->addLayout(statusLayout);
    groupLayout->addWidget(m_statusCard);

    m_configCard->contentLayout()->addLayout(groupLayout);
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
}  // namespace Monitoring
