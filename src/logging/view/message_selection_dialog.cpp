#include "message_selection_dialog.hpp"

#include <stdio.h>

#include <QIcon>
#include <iostream>

#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "logging/view/components/action_button.hpp"

namespace Logging {

// Constructs the message selection dialog for logging configuration
MessageSelectionDialog::MessageSelectionDialog(QWidget* parent)
    : QDialog(parent),
      m_headerWidget(nullptr),
      m_deviceSelector(nullptr),
      m_messagesCard(nullptr),
      m_scrollArea(nullptr),
      m_scrollContent(nullptr),
      m_scrollLayout(nullptr)
{
    setupUi();
}

// Initializes the dialog UI with header, device selector, and scroll area
void MessageSelectionDialog::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Remove window frame to show only the custom dialog
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(false);
    setMinimumSize(635, 517);

    // Main container layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // Apply dialog styling
    const QString dialogStyle = QString(
                                    "QDialog {"
                                    "   background-color: %1;"
                                    "   border: %2px solid %3;"
                                    "   border-radius: %4px;"
                                    "}")
                                    .arg(colors.surfaceMain.name())
                                    .arg(spacing.borderThin)
                                    .arg(colors.borderStrong.name())
                                    .arg(spacing.radiusMd);
    setStyleSheet(dialogStyle);

    // ===== Header Section =====
    m_headerWidget = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                     spacing.spacingMd);
    headerLayout->setSpacing(spacing.spacingMd);

    // Title
    auto* titleLabel = new QLabel("Select Messages to Log", m_headerWidget);
    const QString titleStyle = QString(
                                   "QLabel {"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 24px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textPrimary.name());
    titleLabel->setStyleSheet(titleStyle);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // Close button
    auto* closeBtn = new QPushButton("×", m_headerWidget);
    closeBtn->setFixedSize(48, 48);
    const QString closeBtnStyle = QString(
                                      "QPushButton {"
                                      "   background-color: transparent;"
                                      "   border: none;"
                                      "   font-size: 32px;"
                                      "   font-weight: %1;"
                                      "   color: %2;"
                                      "}"
                                      "QPushButton:hover {"
                                      "   background-color: %3;"
                                      "   border-radius: 24px;"
                                      "}"
                                      "QPushButton:pressed {"
                                      "   background-color: %4;"
                                      "}")
                                      .arg(spacing.fontWeightBold)
                                      .arg(colors.textPrimary.name())
                                      .arg(QColor(0, 0, 0, 13).name(QColor::HexArgb))
                                      .arg(QColor(0, 0, 0, 26).name(QColor::HexArgb));
    closeBtn->setStyleSheet(closeBtnStyle);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(m_headerWidget);

    // ===== Interface Selector =====
    auto* interfaceContainer = new QWidget(this);
    interfaceContainer->setMinimumHeight(94);
    auto* interfaceLayout = new QVBoxLayout(interfaceContainer);
    interfaceLayout->setContentsMargins(spacing.spacingLg + 1, spacing.spacingLg + 1,
                                        spacing.spacingLg + 1, spacing.spacingMd);
    interfaceLayout->setSpacing(spacing.spacingSm);

    // Interface label
    auto* interfaceLabel = new QLabel("Interface", interfaceContainer);
    const QString interfaceLabelStyle = QString(
                                            "QLabel {"
                                            "   border: none;"
                                            "   font-family: 'Roboto';"
                                            "   font-size: %1px;"
                                            "   font-weight: %2;"
                                            "   color: %3;"
                                            "}")
                                            .arg(spacing.fontSizeLg)
                                            .arg(spacing.fontWeightNormal)
                                            .arg(colors.textPrimary.name());
    interfaceLabel->setStyleSheet(interfaceLabelStyle);
    interfaceLayout->addWidget(interfaceLabel);

    // Device selector (combo box styled as filter bar)
    m_deviceSelector = new QComboBox(interfaceContainer);
    m_deviceSelector->setMinimumHeight(34);
    const QString comboStyle = QString(
                                   "QComboBox {"
                                   "   background-color: %1;"
                                   "   border: none;"
                                   "   border-radius: 17px;"
                                   "   padding: 1px %2px;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: %3px;"
                                   "   font-weight: %4;"
                                   "   color: %5;"
                                   "   min-width: 200px;"
                                   "}"
                                   "QComboBox:hover {"
                                   "   background-color: %6;"
                                   "}"
                                   "QComboBox::drop-down {"
                                   "   border: none;"
                                   "   width: %7px;"
                                   "}"
                                   "QComboBox::down-arrow {"
                                   "   image: none;"
                                   "   border: none;"
                                   "   width: %2px;"
                                   "   height: 9px;"
                                   "}"
                                   "QComboBox QAbstractItemView {"
                                   "   background-color: %8;"
                                   "   border: %9px solid %10;"
                                   "   border-radius: %11px;"
                                   "   padding: %12px;"
                                   "   selection-background-color: %1;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: %3px;"
                                   "}")
                                   .arg(colors.surfacePrimary.name())
                                   .arg(spacing.spacingLg)
                                   .arg(spacing.fontSizeMd)
                                   .arg(spacing.fontWeightNormal)
                                   .arg(colors.textSecondary.name())
                                   .arg(colors.surfaceHover.name())
                                   .arg(spacing.spacingXl)
                                   .arg(colors.surfaceMain.name())
                                   .arg(spacing.borderThin)
                                   .arg(colors.borderSubtle.name())
                                   .arg(spacing.radiusMd)
                                   .arg(spacing.spacingXs + 1);
    m_deviceSelector->setStyleSheet(comboStyle);
    m_deviceSelector->setPlaceholderText("Select interface...");
    interfaceLayout->addWidget(m_deviceSelector);

    // Add border to interface container
    const QString containerStyle = QString(
                                       "QWidget {"
                                       "   border: %1px solid %2;"
                                       "   border-radius: %3px;"
                                       "   background-color: %4;"
                                       "}")
                                       .arg(spacing.borderThin)
                                       .arg(colors.borderSubtle.name())
                                       .arg(spacing.radiusMd)
                                       .arg(colors.surfaceMain.name());
    interfaceContainer->setStyleSheet(containerStyle);

    mainLayout->addWidget(interfaceContainer);

    // ===== Messages Card Widget =====
    m_messagesCard =
        new Core::CardWidget("Messages", QString(), QString(":/assets/icon/messages.svg"), this);

    if (auto* messagesCardLayout = m_messagesCard->contentLayout())
    {
        // ===== Scrollable Message Cards Area =====
        m_scrollArea = new QScrollArea(m_messagesCard);
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_scrollArea->setFrameShape(QFrame::NoFrame);

        m_scrollContent = new QWidget(m_scrollArea);
        m_scrollContent->setObjectName("scrollContent");
        m_scrollContent->setStyleSheet(QString("QWidget#scrollContent { background-color: %1; }")
                                           .arg(colors.surfaceMain.name()));

        m_scrollLayout = new QVBoxLayout(m_scrollContent);
        m_scrollLayout->setContentsMargins(0, 0, 0, 0);
        m_scrollLayout->setSpacing(spacing.spacingSm);
        m_scrollLayout->addStretch();

        m_scrollArea->setWidget(m_scrollContent);
        messagesCardLayout->addWidget(m_scrollArea);
    }

    mainLayout->addWidget(m_messagesCard, 1);

    // ===== Bottom Bar with Start Button =====
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);
    bottomLayout->addStretch();

    auto* startBtn = new ActionButton(bottomBar);
    startBtn->setRecordingState(false);  // Start state (not recording)
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
    // Clear message cards vector
    m_messageCards.clear();

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
    for (const auto& msgDef : config.messageDefinitions)
    {
        std::cout << "Adding message card for ID 0x" << std::hex << msgDef.messageId << ": "
                  << msgDef.messageName << " with " << msgDef.signalDescriptions.size()
                  << " signals" << std::endl;
        // Configure the card for logging/selection mode
        Core::DbcMessageCard::Config cardConfig;
        cardConfig.showCheckbox = true;
        cardConfig.startExpanded = false;
        cardConfig.checkboxTooltip = tr("Select message for logging");

        // Create message card (parent is dialog, will be reparented when added to layout)
        auto* card = new Core::DbcMessageCard(
            QString::fromStdString(msgDef.messageName), msgDef.messageId,
            static_cast<int>(msgDef.signalDescriptions.size()), cardConfig, this);

        // Add signal rows with selection mode
        for (const auto& sigDef : msgDef.signalDescriptions)
        {
            Core::DbcSignalRowWidget::Config signalConfig;
            signalConfig.mode = Core::DbcSignalRowWidget::Mode::Selection;
            signalConfig.showSelectionCheckbox = true;
            signalConfig.showRange = false;

            auto* signalRow = new Core::DbcSignalRowWidget(
                QString::fromStdString(sigDef.signalName), QString::fromStdString(sigDef.unit),
                sigDef.minimum, sigDef.maximum, signalConfig, card);

            card->addSignalRow(signalRow);
        }

        // Update header state based on signals
        card->updateHeaderFromSignals();

        // Store reference to the card with message ID as key
        m_messageCards[msgDef.messageId] = card;

        // Add card to layout
        int insertPos = m_scrollLayout->count() - 1;
        if (insertPos < 0)
        {
            insertPos = 0;
        }
        m_scrollLayout->insertWidget(insertPos, card);
    }
}

// Returns map of message IDs to their selected signal names
std::map<uint32_t, QStringList> MessageSelectionDialog::getSelectedSignals() const
{
    std::map<uint32_t, QStringList> selectedSignalsMap;

    for (const auto& [messageId, card] : m_messageCards)
    {
        if (!card || !card->headerCheckbox())
        {
            continue;
        }

        // Only include messages with at least partial selection
        if (card->headerCheckbox()->checkState() == Qt::Unchecked)
        {
            continue;
        }

        QStringList selectedSignals;

        // Collect selected signals from signal rows
        const auto signalRows = card->findChildren<Core::DbcSignalRowWidget*>();
        for (const auto* signalRow : signalRows)
        {
            if (const auto* checkbox = signalRow->selectionCheckbox())
            {
                if (checkbox->isChecked())
                {
                    // Get signal name from the row's name label
                    const auto labels = signalRow->findChildren<QLabel*>();
                    if (!labels.isEmpty())
                    {
                        // First label is typically the signal name
                        selectedSignals.append(labels.first()->text());
                    }
                }
            }
        }

        if (!selectedSignals.isEmpty())
        {
            selectedSignalsMap[messageId] = selectedSignals;
        }
    }

    return selectedSignalsMap;
}

}  // namespace Logging
