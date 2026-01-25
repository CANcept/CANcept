#include "raw_sending_subview.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/widgets/styled_line_edit.hpp"
#include "sending/constants.hpp"

namespace Sending {

RawSendingSubView::RawSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_frameCard(nullptr),
      m_canIdEditor(nullptr),
      m_messageDataEditor(nullptr),
      m_sendButton(nullptr)
{
    setupUi();
}

void RawSendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

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

    // CAN-Bus Configuration Card
    m_configCard = new CanBusConfigCard(true, true, this);
    contentLayout->addWidget(m_configCard);

    // CAN Frame Card
    m_frameCard = new Core::CardWidget(tr("CAN Frame"),
                                       tr("Enter CAN ID and message data as hexadecimal values"),
                                       QString(Constants::CAN_FRAME_ICON_PATH), this);
    auto* frameCardLayout = m_frameCard->contentLayout();

    // CAN ID Input
    auto* canIdLabel = new QLabel(tr("CAN ID (Hexadecimal)"), m_frameCard);
    canIdLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_canIdEditor = new Core::StyledLineEdit(m_frameCard);
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

    m_messageDataEditor = new Core::StyledLineEdit(m_frameCard);
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

    m_sendButton = new SendMessageButton(buttonContainer);
    buttonLayout->addWidget(m_sendButton);

    // Position button container at the bottom of the main layout
    mainLayout->addWidget(buttonContainer);
}

void RawSendingSubView::setAvailableInterfaces(const std::vector<std::string>& interfaces)
{
    if (m_configCard)
    {
        m_configCard->setAvailableInterfaces(interfaces);
    }
}

void RawSendingSubView::setAvailableBaudRates(const std::vector<uint32_t>& baudRates)
{
    if (m_configCard)
    {
        m_configCard->setAvailableBaudRates(baudRates);
    }
}

}  // namespace Sending
