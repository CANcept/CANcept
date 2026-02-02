//
// Message Selection Dialog implementation
//

#include "message_selection_dialog.hpp"

#include <QIcon>
#include <QTimer>

namespace Logging {

// Constructs the message selection dialog for logging configuration
MessageSelectionDialog::MessageSelectionDialog(QWidget* parent)
    : QDialog(parent),
      m_headerWidget(nullptr),
      m_deviceSelector(nullptr),
      m_scrollArea(nullptr),
      m_scrollContent(nullptr),
      m_scrollLayout(nullptr)
{
    setupUi();
}

// Initializes the dialog UI with header, device selector, and scroll area
void MessageSelectionDialog::setupUi()
{
    // Remove window frame to show only the custom dialog
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(false);
    setMinimumSize(635, 517);

    // Main container layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Apply dialog styling
    setStyleSheet(
        "QDialog {"
        "   background-color: white;"
        "   border: 1px solid rgba(0, 0, 0, 0.5);"
        "   border-radius: 10px;"
        "}");

    // ===== Header Section =====
    m_headerWidget = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerLayout->setSpacing(10);

    // Title
    auto* titleLabel = new QLabel("Select Messages to Log", m_headerWidget);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 24px;"
        "   font-weight: 500;"
        "   color: black;"
        "}");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // Close button
    auto* closeBtn = new QPushButton("×", m_headerWidget);
    closeBtn->setFixedSize(48, 48);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   font-size: 32px;"
        "   font-weight: bold;"
        "   color: #1e1e1e;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(0, 0, 0, 0.05);"
        "   border-radius: 24px;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(0, 0, 0, 0.1);"
        "}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(m_headerWidget);

    // ===== Interface Selector =====
    auto* interfaceContainer = new QWidget(this);
    interfaceContainer->setMinimumHeight(94);
    auto* interfaceLayout = new QVBoxLayout(interfaceContainer);
    interfaceLayout->setContentsMargins(17, 17, 17, 10);
    interfaceLayout->setSpacing(8);

    // Interface label
    auto* interfaceLabel = new QLabel("Interface", interfaceContainer);
    interfaceLabel->setStyleSheet(
        "QLabel {"
        "   border: none;"
        "   font-family: 'Roboto';"
        "   font-size: 16px;"
        "   font-weight: 400;"
        "   color: black;"
        "}");
    interfaceLayout->addWidget(interfaceLabel);

    // Device selector (combo box styled as filter bar)
    m_deviceSelector = new QComboBox(interfaceContainer);
    m_deviceSelector->setMinimumHeight(34);
    m_deviceSelector->setStyleSheet(
        "QComboBox {"
        "   background-color: #f3f3f5;"
        "   border: none;"
        "   border-radius: 17px;"
        "   padding: 1px 16px;"
        "   font-family: 'Roboto';"
        "   font-size: 14px;"
        "   font-weight: 400;"
        "   color: #5a5a5a;"
        "   min-width: 200px;"
        "}"
        "QComboBox:hover {"
        "   background-color: #e8e8ea;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 24px;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border: none;"
        "   width: 16px;"
        "   height: 9px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: white;"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   selection-background-color: #f3f3f5;"
        "   font-family: 'Roboto';"
        "   font-size: 14px;"
        "}");
    m_deviceSelector->setPlaceholderText("Select interface...");
    interfaceLayout->addWidget(m_deviceSelector);

    // Add border to interface container
    interfaceContainer->setStyleSheet(
        "QWidget {"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 10px;"
        "   background-color: white;"
        "}");

    mainLayout->addWidget(interfaceContainer);

    // ===== Scrollable Message Cards Area =====
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setMinimumHeight(263);
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 10px;"
        "   background-color: white;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background-color: #f3f3f5;"
        "   width: 10px;"
        "   border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #c0c0c0;"
        "   border-radius: 5px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #a0a0a0;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   border: none;"
        "   background: none;"
        "   height: 0px;"
        "}");

    m_scrollContent = new QWidget();
    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    m_scrollLayout->setContentsMargins(10, 10, 10, 10);
    m_scrollLayout->setSpacing(10);

    m_scrollLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea, 1);

    // ===== Bottom Bar with Start Button =====
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);
    bottomLayout->addStretch();

    auto* startBtn = new QPushButton(bottomBar);
    startBtn->setText(" Start");
    startBtn->setIcon(QIcon(":/assets/icon/sending/send.svg"));
    startBtn->setIconSize(QSize(20, 20));
    startBtn->setFixedSize(150, 75);
    startBtn->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   border-radius: 30px;"
        "   font-family: 'Roboto';"
        "   font-size: 22px;"
        "   font-weight: 500;"
        "   padding: 10px 10px;"
        "   background-color: #f3f3f5;"
        "   color: black;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e8e8ea;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #d8d8da;"
        "}");
    connect(startBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(startBtn);

    mainLayout->addWidget(bottomBar);
}

// Populates the device selector dropdown with available CAN interfaces
void MessageSelectionDialog::setAvailableDevices(const QStringList& devices)
{
    m_deviceSelector->clear();
    m_deviceSelector->addItems(devices);
}

// Returns the currently selected CAN device/interface
QString MessageSelectionDialog::getSelectedDevice() const
{
    return m_deviceSelector->currentText();
}

// Adds a message card widget to the scrollable list
void MessageSelectionDialog::addMessageCard(Core::DbcMessageCard* card)
{
    if (!card)
    {
        return;
    }

    // Insert before the stretch
    int insertPos = m_scrollLayout->count() - 1;
    if (insertPos < 0)
    {
        insertPos = 0;
    }
    m_scrollLayout->insertWidget(insertPos, card);
}

// Removes all message cards
void MessageSelectionDialog::clearCards()
{
    // Clear signal widgets map
    m_signalWidgets.clear();

    // Remove all widgets except the stretch
    while (m_scrollLayout->count() > 1)
    {
        QLayoutItem* item = m_scrollLayout->takeAt(0);
        if (item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

// Populates dialog with messages from parsed DBC configuration
void MessageSelectionDialog::setDbcConfig(const Core::DbcConfig& config)
{
    // Clear existing cards
    clearCards();

    // Create a card for each message in the DBC config
    for (const auto& message : config.messageDefinitions)
    {
        QWidget* messageCard = createMessageCardWithSignals(message);

        int insertPos = m_scrollLayout->count() - 1;
        if (insertPos < 0)
        {
            insertPos = 0;
        }
        m_scrollLayout->insertWidget(insertPos, messageCard);
    }
}

// Creates a message card widget with embedded signal selection
QWidget* MessageSelectionDialog::createMessageCardWithSignals(
    const Core::DbcMessageDescription& message)
{
    // Configure the DBC message card for logging selection mode
    Core::DbcMessageCard::Config cardConfig;
    cardConfig.showCheckbox = true;
    cardConfig.startExpanded = false;
    cardConfig.checkboxTooltip = "Select message for logging";

    // Create container widget to hold both the card and signal selection
    auto* containerWidget = new QWidget(m_scrollContent);
    auto* containerLayout = new QVBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(8);

    // Create the DBC message card
    auto* messageCard = new Core::DbcMessageCard(
        QString::fromStdString(message.messageName),
        message.messageId,
        static_cast<int>(message.signalDescriptions.size()),
        cardConfig,
        containerWidget);
    
    containerLayout->addWidget(messageCard);

    // Create signal selection widget (displayed below the card when expanded)
    auto* signalWidget = new SignalSelectionWidget(
        message.messageId, 
        QString::fromStdString(message.messageName), 
        containerWidget);
    signalWidget->setSignals(message.signalDescriptions);
    signalWidget->setVisible(false);  // Initially hidden

    // Store reference to the signal widget
    m_signalWidgets[message.messageId] = signalWidget;
    
    containerLayout->addWidget(signalWidget);

    // Connect card expansion to show/hide signal selection
    connect(messageCard, &QWidget::customContextMenuRequested, this, [signalWidget, messageCard]() {
        signalWidget->setVisible(messageCard->isExpanded());
    });

    // Simple workaround: monitor the card's expand button state
    // When card expands, show signal widget
    auto* expandMonitorTimer = new QTimer(containerWidget);
    expandMonitorTimer->setInterval(100);
    connect(expandMonitorTimer, &QTimer::timeout, [messageCard, signalWidget]() {
        signalWidget->setVisible(messageCard->isExpanded());
    });
    expandMonitorTimer->start();

    return containerWidget;
}

// Returns map of message IDs to their selected signal names
std::map<uint32_t, QStringList> MessageSelectionDialog::getSelectedSignals() const
{
    std::map<uint32_t, QStringList> selectedSignalsMap;

    for (const auto& [messageId, widget] : m_signalWidgets)
    {
        QStringList selectedSignals = widget->getSelectedSignals();
        if (!selectedSignals.isEmpty())
        {
            selectedSignalsMap[messageId] = selectedSignals;
        }
    }

    return selectedSignalsMap;
}

}  // namespace Logging
