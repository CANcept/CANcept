#include "can_bus_config_card.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "monitoring/constants.hpp"

namespace Monitoring {

CanBusConfigCard::CanBusConfigCard(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCard(nullptr),
      m_frameRateCard(nullptr),
      m_messageCountCard(nullptr),
      m_interfaceCombo(nullptr),
      m_connectionButton(nullptr)
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

    // TODO Implement this code into class
    m_interfaceCard = new Core::CardWidget(nullptr, nullptr, Constants::CAN_CONFIGURATION_ICON_PATH, this);  // No title here, we'll add a custom one
    m_interfaceCard->setStyleSheet(
        "QGroupBox { border: 1px solid #C0C0C0; border-radius: 8px; }");
    auto* groupLayout = new QVBoxLayout(m_interfaceCard);

    // --- Row 1: Config & Connect ---
    auto* topRow = new QHBoxLayout();

    // Title with Icon
    m_titleIcon = new QLabel(this);
    m_titleIcon->setPixmap(
        QPixmap(Constants::CAN_CONFIGURATION_ICON_PATH).scaled(24, 24, Qt::KeepAspectRatio));
    auto* titleLabel = new QLabel("CAN-Bus Connection", this);
    titleLabel->setStyleSheet("font-size: 20px;");

    m_dbcCheck = new QCheckBox(this);
    m_dbcCheck->setText("DBC mode");

    m_interfaceCombo = new Core::StyledComboBox(this);
    m_interfaceCombo->setStyleSheet(
        QString("background-color: %1; width: 300px; border-radius: 15px; font-size: 15px;")
            .arg(THEME.colors().surfacePrimary.name()));
    m_interfaceCombo->addItems({"vcan0", "can0", "can1"});

    m_connectButton = new QPushButton("Connect", this);
    m_connectButton->setIcon(QIcon(Constants::BUS_CONNECT_BUTTON_ICON_PATH));  // Initial state
    m_connectButton->setStyleSheet(
        QString("background-color: %1; width: 100px; border-radius: 15px; font-size: 15px;")
            .arg(THEME.colors().surfacePrimary.name()));

    topRow->addWidget(m_titleIcon);
    topRow->addWidget(titleLabel);
    topRow->addStretch();
    topRow->addSpacing(30);
    topRow->addWidget(m_interfaceCombo);
    topRow->addSpacing(30);
    topRow->addWidget(m_connectButton);

    // --- Row 2: Status Boxes ---
    auto* bottomRow = new QHBoxLayout();

    // Create the three boxes
    QFrame* statusBox = createStatBox("Status", m_statusValueLabel = new QLabel("Disconnected"));
    QFrame* fpsBox = createStatBox("Frame rate", m_fpsValueLabel = new QLabel("0 fps"));
    QFrame* msgBox = createStatBox("Messages", m_msgCountValueLabel = new QLabel("0 messages"));

    // Set initial status color
    m_statusValueLabel->setStyleSheet("font-size: 15px; color: red;");
    m_fpsValueLabel->setStyleSheet("font-size: 15px; color: gray;");
    m_msgCountValueLabel->setStyleSheet("font-size: 15px; color: gray;");

    bottomRow->addWidget(statusBox, 1);  // The '1' makes them share space equally
    bottomRow->addWidget(fpsBox, 1);
    bottomRow->addWidget(msgBox, 1);

    // Add rows to group
    groupLayout->addLayout(topRow);
    groupLayout->addLayout(bottomRow);

    //Wire up Button Functionality
    connect(m_connectButton, &QPushButton::clicked, this, [this]() -> void {
    if (m_connectButton->text() == "Connect")
    {
        // Switch to Connected state
        m_connectButton->setText("Disconnect");
        m_connectButton->setIcon(QIcon(Constants::BUS_DISCONNECT_BUTTON_ICON_PATH));
        m_connectButton->setStyleSheet(QString("background-color: %1; color: black; width: "
                                               "100px; border-radius: 15px; font-size: 15px;")
                                           .arg(THEME.colors().statusWarning.name()));

        m_statusValueLabel->setText(
            QString("Connected (%1)").arg(m_interfaceCombo->currentText()));
        m_statusValueLabel->setStyleSheet("font-size: 15px; color: green;");
    } else
    {
        // Switch to Disconnected state
        m_connectButton->setText("Connect");
        m_connectButton->setIcon(QIcon(Constants::BUS_CONNECT_BUTTON_ICON_PATH));
        m_connectButton->setStyleSheet(QString("background-color: %1; color: black; width: "
                                               "100px; border-radius: 15px; font-size: 15px;")
                                           .arg(THEME.colors().surfacePrimary.name()));

        m_statusValueLabel->setText("Disconnected");
        m_statusValueLabel->setStyleSheet("font-size:15px; color: red;");
    }
});


    //---OLD CODE
    auto* innerCardsRow = new QHBoxLayout();
    innerCardsRow->setSpacing(spacing.spacingLg);

    m_interfaceCard =
        new Core::CardWidget(QString(), Constants::INTERFACE_LABEL, QString(), m_configCard);
    m_interfaceCombo = new Core::StyledComboBox(m_interfaceCard);
    m_interfaceCombo->setPlaceholderText(Constants::INTERFACE_PLACEHOLDER);
    m_interfaceCard->contentLayout()->addWidget(m_interfaceCombo);
    m_connectionButton = new QPushButton(Constants::BUS_CONNECT_BUTTON_LABEL, m_interfaceCard);
    m_connectionButton->setIcon(QIcon(Constants::BUS_CONNECT_BUTTON_ICON_PATH));  // Initial state
    m_connectionButton->setStyleSheet(
        QString("background-color: %1; width: 100px; border-radius: 15px; font-size: 15px;")
            .arg(THEME.colors().surfacePrimary.name()));
    m_interfaceCard->contentLayout()->addWidget(m_connectionButton);
    innerCardsRow->addWidget(m_interfaceCard);

    /*m_statusCard =
        new Core::CardWidget(QString(), Constants::CAN_STATUS_LABEL, QString(), m_configCard);
    m_messageCountCard->contentLayout()->addWidget(
        new QLabel(Constants::CAN_CONFIG_DISCONNECTED_LABEL, m_messageCountCard));
    innerCardsRow->addWidget(m_statusCard);

    m_messageCountCard =
        new Core::CardWidget(QString(), Constants::MESSAGE_COUNT_LABEL, QString(), m_configCard);
    m_messageCountCard->contentLayout()->addWidget(
        new QLabel(Constants::MESSAGE_COUNT_PLACEHOLDER, m_messageCountCard));
    innerCardsRow->addWidget(m_messageCountCard);

    m_frameRateCard =
        new Core::CardWidget(QString(), Constants::FRAME_RATE_LABEL, QString(), m_configCard);
    m_frameRateCard->contentLayout()->addWidget(
        new QLabel(Constants::FRAME_RATE_PLACEHOLDER, m_frameRateCard));
    innerCardsRow->addWidget(m_frameRateCard);
    */

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

// Helper function to keep code clean
auto CanBusConfigCard::createStatBox(const ::QString& title, QLabel*& valueLabel)
    -> QFrame*
{
    auto* frame = new QFrame(this);
    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    frame->setFrameStyle(QFrame::StyledPanel);
    frame->setStyleSheet(
        "QFrame { border: 1px solid #C0C0C0; border-radius: 8px; }"
        "QLabel { border: none; background: transparent; }");

    auto* layout = new QVBoxLayout(frame);

    layout->setContentsMargins(20, 5, 5, 10);
    layout->setSpacing(5);

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px;");

    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return frame;
}

}  // namespace Monitoring
