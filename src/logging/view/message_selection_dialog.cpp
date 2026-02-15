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

    // Use standard dialog window with system frame
    setWindowTitle("Select Messages to Log");
    setModal(true);
    setMinimumSize(635, 517);

    // Main container layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // Apply background styling
    const QString dialogStyle = QString(
                                    "QDialog {"
                                    "   background-color: %1;"
                                    "}")
                                    .arg(colors.surfaceMain.name());
    setStyleSheet(dialogStyle);

    // ===== Messages Card Widget =====
    m_messagesCard = new Core::CardWidget("Messages", QString(),
                                          QString(":/assets/icon/sending/messages.svg"), this);

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
