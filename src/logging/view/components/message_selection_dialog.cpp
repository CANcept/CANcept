#include "message_selection_dialog.hpp"

#include <QRadioButton>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "logging/view/components/start_stop_button.hpp"

namespace Logging {

// Constructs the message selection dialog for logging configuration
MessageSelectionDialog::MessageSelectionDialog(QWidget* parent)
    : QDialog(parent),
      m_headerWidget(nullptr),
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
    setModal(true);  // Block interaction with parent window
    setMinimumWidth(600);
    setMinimumHeight(700);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    // Main container layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // ===== Header Section =====
    m_headerWidget = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                     spacing.spacingMd);
    headerLayout->setSpacing(spacing.spacingMd);

    // Title
    m_titleLabel = new QLabel("Select Messages to Log", m_headerWidget);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    // Close button
    m_closeButton = new QPushButton("×", m_headerWidget);
    m_closeButton->setFixedSize(48, 48);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(m_closeButton);

    mainLayout->addWidget(m_headerWidget);

    // Type
    m_buttonWidget = new QWidget();
    auto* buttonLayout = new QHBoxLayout(m_buttonWidget);
    buttonLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                     spacing.spacingMd);
    buttonLayout->setSpacing(spacing.spacingMd);
    m_dbcLabel = new QLabel("DBC based");
    m_rawLabel = new QLabel("Raw");
    m_rawLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_logTypeSwitch = new Core::StyledSwitch();

    buttonLayout->addWidget(m_rawLabel);
    buttonLayout->addWidget(m_logTypeSwitch);
    buttonLayout->addWidget(m_dbcLabel);
    mainLayout->addWidget(m_buttonWidget);
    connect(m_logTypeSwitch, &Core::StyledSwitch::toggled, this,
                &MessageSelectionDialog::onLogTypeToggle);

    // ===== Messages Card Widget =====
    m_messagesCard = new Core::CardWidget("Messages", QString(), QString(), this);

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

        m_scrollLayout = new QVBoxLayout(m_scrollContent);
        m_scrollLayout->setContentsMargins(0, 0, 0, 0);
        m_scrollLayout->setSpacing(spacing.spacingSm);
        m_scrollLayout->addStretch();

        m_scrollArea->setWidget(m_scrollContent);
        messagesCardLayout->addWidget(m_scrollArea);
    }

    mainLayout->addWidget(m_messagesCard, 1);
    QSizePolicy sp_retain = m_messagesCard->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_messagesCard->setSizePolicy(sp_retain);
    m_messagesCard->setVisible(false);

    // ===== Bottom Bar with Start Button =====
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);
    bottomLayout->addStretch();

    auto* startBtn = new StartStopButton(bottomBar);
    startBtn->setRecordingState(false);  // Start state (not recording)
    connect(startBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(startBtn);

    applyStyle();
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

void MessageSelectionDialog::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    setStyleSheet(QString("QDialog {"
                          "   background-color: %1;"
                          "   border: %2px solid %3;"
                          "   border-radius: %4px;"
                          "}")
                      .arg(colors.surfaceMain.name())
                      .arg(spacing.borderThin)
                      .arg(colors.borderStrong.name())
                      .arg(spacing.radiusMd));
    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("QLabel {"
                                            "   font-size: %3px;"
                                            "   font-weight: %1;"
                                            "   color: %2;"
                                            "}")
                                        .arg(spacing.fontWeightMedium)
                                        .arg(colors.textPrimary.name())
                                        .arg(spacing.fontSizeLg));
    }
    if (m_closeButton)
    {
        m_closeButton->setStyleSheet(QString("QPushButton {"
                                             "   background-color: transparent;"
                                             "   border: none;"
                                             "   font-size: %5px;"
                                             "   font-weight: %1;"
                                             "   color: %2;"
                                             "}"
                                             "QPushButton:hover {"
                                             "   background-color: %3;"
                                             "   border-radius: %6px;"
                                             "}"
                                             "QPushButton:pressed {"
                                             "   background-color: %4;"
                                             "}")
                                         .arg(spacing.fontWeightBold)
                                         .arg(colors.textPrimary.name())
                                         .arg(QColor(0, 0, 0, 13).name(QColor::HexArgb))
                                         .arg(QColor(0, 0, 0, 26).name(QColor::HexArgb))
                                         .arg(spacing.fontSizeLg)
                                         .arg(spacing.radiusLg));
    }
    if (m_scrollContent)
    {
        m_scrollContent->setStyleSheet(QString("QWidget#scrollContent { background-color: %1; }")
                                           .arg(colors.surfaceMain.name()));
    }
    if (m_buttonWidget)
    {
        m_buttonWidget->setStyleSheet(QString("QWidget {"
                                              "   border: none;"
                                              "   font-size: %3px;"
                                              "   font-weight: %1;"
                                              "   color: %2;"
                                              "}")
                                          .arg(spacing.fontWeightNormal)
                                          .arg(colors.textSecondary.name())
                                          .arg(spacing.fontSizeSm));
    }
    if (m_dbcLabel)
    {
        m_dbcLabel->setStyleSheet(QString("QLabel {"
                                          "   border: none;"
                                          "   font-size: %3px;"
                                          "   font-weight: %1;"
                                          "   color: %2;"
                                          "}")
                                      .arg(spacing.fontWeightNormal)
                                      .arg(colors.textSecondary.name())
                                      .arg(spacing.fontSizeSm));
    }
    if (m_rawLabel)
    {
        m_rawLabel->setStyleSheet(QString("QLabel {"
                                          "   border: none;"
                                          "   font-size: %3px;"
                                          "   font-weight: %1;"
                                          "   color: %2;"
                                          "}")
                                      .arg(spacing.fontWeightNormal)
                                      .arg(colors.textSecondary.name())
                                      .arg(spacing.fontSizeSm));
    }
}

LogSessionType MessageSelectionDialog::getSelectedLogSessionType() const
{
    if (m_logTypeSwitch)
    {
        return m_logTypeSwitch->isChecked() ? DBC_BASED : RAW;
    }
    return RAW;
}

void MessageSelectionDialog::onLogTypeToggle(bool checked)
{
    if (checked)
    {
        m_messagesCard->setVisible(true);
        //setMinimumHeight(600);
    } else
    {
        m_messagesCard->setVisible(false);
        //setMinimumHeight(0);
    }
}

bool MessageSelectionDialog::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Logging
