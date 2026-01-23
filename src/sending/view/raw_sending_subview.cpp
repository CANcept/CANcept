#include "raw_sending_subview.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

RawSendingSubView::RawSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_interfaceCombo(nullptr),
      m_baudRateCombo(nullptr),
      m_frameCard(nullptr),
      m_canIdEditor(nullptr),
      m_messageDataEditor(nullptr),
      m_sendButton(nullptr)
{
    setupUi();
}

auto RawSendingSubView::createCard(const QString& title, const QString& subtitle) -> QFrame*
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* card = new QFrame(this);
    card->setObjectName("card");

    // Apply modern card styling
    QString cardStyle =
        QString(
            "QFrame#card {"
            "  background-color: %1;"
            "  border: %2px solid %3;"
            "  border-radius: %4px;"
            "}")
            .arg(colors.surfaceMain.name(QColor::HexArgb))  // Includes Alpha as #AARRGGBB
            .arg(spacing.borderThin)
            .arg(colors.borderSubtle.name(QColor::HexArgb))
            .arg(spacing.radiusSm);
    card->setStyleSheet(cardStyle);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    cardLayout->setSpacing(spacing.spacingMd);

    // Card title
    if (!title.isEmpty())
    {
        auto* titleLabel = new QLabel(title, card);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(spacing.fontSizeLg);
        titleFont.setWeight(static_cast<QFont::Weight>(spacing.fontWeightBold));
        titleLabel->setFont(titleFont);
        titleLabel->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));
        cardLayout->addWidget(titleLabel);
    }

    // Card subtitle (optional)
    if (!subtitle.isEmpty())
    {
        auto* subtitleLabel = new QLabel(subtitle, card);
        QFont subtitleFont = subtitleLabel->font();
        subtitleFont.setPointSize(spacing.fontSizeSm);
        subtitleLabel->setFont(subtitleFont);
        subtitleLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
        cardLayout->addWidget(subtitleLabel);
    }

    return card;
}

void RawSendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // Main scroll area for better UX with smaller screens
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* scrollContent = new QWidget();
    scrollArea->setWidget(scrollContent);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);

    auto* contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                      spacing.spacingLg);
    contentLayout->setSpacing(spacing.spacingLg);

    // === CAN-Bus Configuration Card ===
    m_configCard = createCard(tr("CAN-Bus Configuration"));
    auto* configCardLayout = qobject_cast<QVBoxLayout*>(m_configCard->layout());

    auto* configGrid = new QGridLayout();
    configGrid->setHorizontalSpacing(spacing.spacingLg);
    configGrid->setVerticalSpacing(spacing.spacingMd);

    // Interface selector
    auto* interfaceLabel = new QLabel(tr("Interface"), m_configCard);
    interfaceLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    m_interfaceCombo = new QComboBox(m_configCard);
    m_interfaceCombo->setPlaceholderText(tr("Select interface..."));

    configGrid->addWidget(interfaceLabel, 0, 0);
    configGrid->addWidget(m_interfaceCombo, 1, 0);

    // Baud rate selector
    auto* baudRateLabel = new QLabel(tr("Baud Rate"), m_configCard);
    baudRateLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
    m_baudRateCombo = new QComboBox(m_configCard);
    m_baudRateCombo->setPlaceholderText(tr("Select baud rate..."));

    configGrid->addWidget(baudRateLabel, 0, 1);
    configGrid->addWidget(m_baudRateCombo, 1, 1);

    // Make both columns stretch equally
    configGrid->setColumnStretch(0, 1);
    configGrid->setColumnStretch(1, 1);

    configCardLayout->addLayout(configGrid);
    contentLayout->addWidget(m_configCard);

    // === CAN Frame Card ===
    m_frameCard =
        createCard(tr("CAN Frame"), tr("Enter CAN ID and message data as hexadecimal values"));
    auto* frameCardLayout = qobject_cast<QVBoxLayout*>(m_frameCard->layout());

    // CAN ID Input
    auto* canIdLabel = new QLabel(tr("CAN ID (Hexadecimal)"), m_frameCard);
    canIdLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_canIdEditor = new QLineEdit(m_frameCard);
    m_canIdEditor->setPlaceholderText("0x 1A2B");

    // Hex validator for CAN ID (allow up to 8 hex chars for extended IDs)
    QRegularExpression idRegex("^(0[xX])?\\s*[0-9A-Fa-f]{1,8}$");
    auto* idValidator = new QRegularExpressionValidator(idRegex, m_canIdEditor);
    m_canIdEditor->setValidator(idValidator);

    frameCardLayout->addWidget(canIdLabel);
    frameCardLayout->addWidget(m_canIdEditor);

    frameCardLayout->addSpacing(spacing.spacingMd);

    // Message Data Input
    auto* messageDataLabel = new QLabel(tr("Message Data (max 8 bytes)"), m_frameCard);
    messageDataLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_messageDataEditor = new QLineEdit(m_frameCard);
    m_messageDataEditor->setPlaceholderText("01 02 03 04 05 06 07 08");

    // Hex validator for message data (space-separated hex bytes)
    // Allows patterns like: "01 02 03" or "01020304" or mixed
    QRegularExpression dataRegex("^[0-9A-Fa-f\\s]*$");
    auto* dataValidator = new QRegularExpressionValidator(dataRegex, m_messageDataEditor);
    m_messageDataEditor->setValidator(dataValidator);

    frameCardLayout->addWidget(messageDataLabel);
    frameCardLayout->addWidget(m_messageDataEditor);

    contentLayout->addWidget(m_frameCard);

    // Add stretch to push cards to top
    contentLayout->addStretch();

    // === Floating Send Button ===
    // Create a container for the button positioned at bottom right
    auto* buttonContainer = new QWidget(this);
    buttonContainer->setFixedHeight(80);

    auto* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                     spacing.spacingLg);
    buttonLayout->addStretch();

    m_sendButton = new QPushButton(tr("Send Message"), buttonContainer);
    m_sendButton->setMinimumHeight(40);
    m_sendButton->setMinimumWidth(160);

    // Style send button as primary action with elevation
    QString buttonStyle = QString(
                              "QPushButton {"
                              "  background-color: %1;"
                              "  color: %2;"
                              "  border: none;"
                              "  border-radius: %3px;"
                              "  padding: %4px %5px;"
                              "  font-weight: %6;"
                              "  font-size: %7px;"
                              "}"
                              "QPushButton:hover {"
                              "  background-color: %8;"
                              "}"
                              "QPushButton:pressed {"
                              "  background-color: %8;"
                              "}"
                              "QPushButton:disabled {"
                              "  background-color: %9;"
                              "  color: %10;"
                              "}")
                              .arg(colors.colorPrimary.name())
                              .arg(colors.surfaceForeground.name())
                              .arg(spacing.radiusMd)
                              .arg(spacing.spacingMd)
                              .arg(spacing.spacingXl)
                              .arg(spacing.fontWeightMedium)
                              .arg(spacing.fontSizeMd)
                              .arg(colors.colorPrimaryHover.name())
                              .arg(colors.surfaceSecondary.name())
                              .arg(colors.textDisabled.name());
    m_sendButton->setStyleSheet(buttonStyle);
    m_sendButton->setIcon(QIcon(Constants::SEND_BUTTON_ICON_PATH));

    buttonLayout->addWidget(m_sendButton);

    // Position button container at the bottom of the main layout
    mainLayout->addWidget(buttonContainer);
}

void RawSendingSubView::setAvailableInterfaces(const std::vector<std::string>& interfaces)
{
    m_interfaceCombo->clear();
    for (const auto& interface : interfaces)
    {
        m_interfaceCombo->addItem(QString::fromStdString(interface));
    }
}

void RawSendingSubView::setAvailableBaudRates(const std::vector<uint32_t>& baudRates)
{
    m_baudRateCombo->clear();
    for (const auto& rate : baudRates)
    {
        m_baudRateCombo->addItem(QString::number(rate), QVariant(rate));
    }
}

}  // namespace Sending
