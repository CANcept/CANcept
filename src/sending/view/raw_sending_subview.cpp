#include "raw_sending_subview.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "components/hex_id_line_edit.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/styled_line_edit.hpp"
#include "sending/constants.hpp"
#include "validator/hex_data_formatter.hpp"

namespace Sending {

RawSendingSubView::RawSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_frameCard(nullptr),
      m_canIdEditor(nullptr),
      m_messageDataEditor(nullptr),
      m_messageDataFormatter(nullptr),
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
    m_frameCard = new Core::CardWidget(Constants::CAN_FRAME_TITLE, Constants::CAN_FRAME_DESCRIPTION,
                                       Constants::CAN_FRAME_ICON_PATH, this);
    auto* frameCardLayout = m_frameCard->contentLayout();

    // CAN ID Input
    auto* canIdLabel = new QLabel(Constants::CAN_ID_LABEL, m_frameCard);
    canIdLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_canIdEditor = new HexIdLineEdit(m_frameCard);
    frameCardLayout->addWidget(canIdLabel);
    frameCardLayout->addWidget(m_canIdEditor);

    frameCardLayout->addSpacing(spacing.spacingMd);

    // Message Data Input
    auto* messageDataLabel = new QLabel(Constants::MESSAGE_DATA_LABEL, m_frameCard);
    messageDataLabel->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_messageDataEditor = new Core::StyledLineEdit(m_frameCard);
    m_messageDataEditor->setPlaceholderText(Constants::MESSAGE_DATA_PLACEHOLDER);
    frameCardLayout->addWidget(messageDataLabel);
    frameCardLayout->addWidget(m_messageDataEditor);

    // Setup input validation and formatting
    setupCanIdInput();
    setupMessageDataInput();
    contentLayout->addWidget(m_frameCard);
    contentLayout->addStretch();

    // Send Message button in the bottom right corner
    auto* buttonContainer = new QWidget(this);
    buttonContainer->setFixedHeight(Constants::BUTTON_CONTAINER_HEIGHT);

    auto* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                     spacing.spacingLg);
    buttonLayout->addStretch();

    m_sendButton = new SendMessageButton(buttonContainer);
    buttonLayout->addWidget(m_sendButton);
    mainLayout->addWidget(buttonContainer);
}

void RawSendingSubView::setAvailableInterfaces(const std::vector<std::string>& interfaces) const
{
    if (m_configCard)
    {
        m_configCard->setAvailableInterfaces(interfaces);
    }
}

void RawSendingSubView::setAvailableBaudRates(const std::vector<uint32_t>& baudRates) const
{
    if (m_configCard)
    {
        m_configCard->setAvailableBaudRates(baudRates);
    }
}

void RawSendingSubView::setupCanIdInput() const
{
    if (!m_canIdEditor)
    {
        return;
    }
    // Configure for standard CAN ID (0x000 - 0x7FF)
    m_canIdEditor->setMaxHexValue(Constants::MAX_CAN_ID);
}

void RawSendingSubView::setupMessageDataInput()
{
    if (!m_messageDataEditor)
    {
        return;
    }
    m_messageDataFormatter = new HexDataFormatter(m_messageDataEditor, Constants::MAX_CAN_DLC);
}

}  // namespace Sending
