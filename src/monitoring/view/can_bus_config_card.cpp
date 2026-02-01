#include "can_bus_config_card.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "monitoring/constants.hpp"

namespace Monitoring {

CanBusConfigCard::CanBusConfigCard(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCard(nullptr),
      m_frameRateCard(nullptr),
      m_messageCountCard(nullptr),
      m_interfaceCombo(nullptr)
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

    auto* groupLayout = new QHBoxLayout(m_configCard);
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
    auto* statusLayout = new QHBoxLayout(m_statusCard);
    statusLayout->setSpacing(spacing.spacingSm);
    m_fpsValueLabel = new QLabel(m_statusCard);
    m_fpsValueLabel->setText(QString("Frames/s: ") + Constants::FRAME_RATE_PLACEHOLDER);
    statusLayout->addWidget(m_fpsValueLabel);
    m_msgCountValueLabel = new QLabel(m_statusCard);
    m_msgCountValueLabel->setText(QString("Message types: ") +
                                  Constants::MESSAGE_COUNT_PLACEHOLDER);
    statusLayout->addWidget(m_msgCountValueLabel);
    m_statusCard->contentLayout()->addLayout(statusLayout);
    groupLayout->addWidget(m_statusCard);

    m_dbcToggleButton = new QPushButton(m_configCard);
    m_dbcToggleButton->setCheckable(true);
    m_dbcToggleButton->setText("Raw Mode");
    m_dbcToggleButton->setFixedSize(100, 32);
    m_dbcToggleButton->setCursor(Qt::PointingHandCursor);
    m_dbcToggleButton->setStyleSheet(
        "QPushButton {"
        "background-color: #f5f5f5;"
        "border: none;"
        "border-radius: 16px;"
        "color: #404040;"
        "font-weight: bold;"
        "padding: 0px;"
        "}"
        "QPushButton:checked {"
        "background-color: #e8e8e8;"
        "color: #404040;"
        "}");

    connect(m_dbcToggleButton, &QPushButton::toggled, this, [this](bool checked) {
        m_dbcToggleButton->setText(checked ? "DBC Mode" : "Raw Mode");
        LOG_INF("MonitoringComponent", "Monitoring mode changed");
    });

    m_configCard->contentLayout()->addWidget(m_dbcToggleButton);
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
