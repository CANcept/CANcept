#include "dbc_based_sending_subview.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

DbcSendingSubView::DbcSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_configCard(nullptr),
      m_listHeader(nullptr),
      m_scrollArea(nullptr),
      m_scrollContent(nullptr),
      m_cardsLayout(nullptr),
      m_sendButton(nullptr)
{
    setupUi();
}

void DbcSendingSubView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    // === CAN-Bus Configuration Card (Interface only, no Baud Rate) ===
    m_configCard = new CanBusConfigCard(true, false, this);
    mainLayout->addWidget(m_configCard);

    // === Messages Header ===
    m_listHeader = new QLabel(Constants::MESSAGES_LABEL, this);
    m_listHeader->setStyleSheet(QString("font-weight: %1; font-size: %2px;")
                                    .arg(spacing.fontWeightBold)
                                    .arg(spacing.fontSizeMd));
    mainLayout->addWidget(m_listHeader);

    // === Scroll Area for Message Cards ===
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollContent = new QWidget(m_scrollArea);
    m_cardsLayout = new QVBoxLayout(m_scrollContent);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(spacing.spacingSm);
    m_cardsLayout->addStretch();  // Push cards to top

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea, 1);  // Give scroll area stretch priority

    // === Footer with Send Button ===
    auto* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, spacing.spacingSm, 0, 0);

    m_sendButton = new SendMessageButton(this);

    footerLayout->addStretch();
    footerLayout->addWidget(m_sendButton);

    mainLayout->addLayout(footerLayout);
}

void DbcSendingSubView::addMessageCard(Core::DbcMessageCard* card)
{
    if (card && m_cardsLayout)
    {
        // Insert before the stretch
        int insertIndex = m_cardsLayout->count() - 1;
        if (insertIndex < 0)
        {
            insertIndex = 0;
        }
        m_cardsLayout->insertWidget(insertIndex, card);
    }
}

void DbcSendingSubView::clearMessages()
{
    if (!m_cardsLayout)
    {
        return;
    }

    // Remove all widgets except the stretch at the end
    while (m_cardsLayout->count() > 1)
    {
        QLayoutItem* item = m_cardsLayout->takeAt(0);
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
}

void DbcSendingSubView::setAvailableInterfaces(const std::vector<std::string>& interfaces)
{
    if (m_configCard)
    {
        m_configCard->setAvailableInterfaces(interfaces);
    }
}

}  // namespace Sending
