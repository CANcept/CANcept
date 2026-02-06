#include "dbc_based_sending_subview.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "sending/constants.hpp"
#include "sending/model/sending_model.hpp"

namespace Sending {

DbcSendingSubView::DbcSendingSubView(QWidget* parent)
    : QWidget(parent),
      m_outerScrollArea(nullptr),
      m_messagesCard(nullptr),
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

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_outerScrollArea = new QScrollArea(this);
    m_outerScrollArea->setWidgetResizable(true);
    m_outerScrollArea->setFrameShape(QFrame::NoFrame);
    m_outerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* scrollContent = new QWidget(m_outerScrollArea);
    m_outerScrollArea->setWidget(scrollContent);

    auto* mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    m_messagesCard = new Core::CardWidget(Constants::MESSAGES_LABEL, QString(),
                                          QString(Constants::MESSAGES_ICON_PATH), scrollContent);
    if (auto* messagesCardLayout = m_messagesCard->contentLayout())
    {
        m_scrollArea = new QScrollArea(m_messagesCard);
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setFrameShape(QFrame::NoFrame);

        m_scrollContent = new QWidget(m_scrollArea);
        m_scrollContent->setObjectName("scrollContent");
        m_cardsLayout = new QVBoxLayout(m_scrollContent);
        m_cardsLayout->setContentsMargins(0, 0, 0, 0);
        m_cardsLayout->setSpacing(spacing.spacingSm);
        m_cardsLayout->addStretch();

        m_scrollArea->setWidget(m_scrollContent);
        messagesCardLayout->addWidget(m_scrollArea);
    }

    mainLayout->addWidget(m_messagesCard, 1);

    outerLayout->addWidget(m_outerScrollArea, 1);

    // Footer outside the scroll area
    auto* footerContainer = new QWidget(this);
    auto* footerLayout = new QHBoxLayout(footerContainer);
    footerLayout->setContentsMargins(spacing.spacingLg, spacing.spacingSm, spacing.spacingLg,
                                     spacing.spacingLg);

    m_sendButton = new SendMessageButton(footerContainer);

    footerLayout->addStretch();
    footerLayout->addWidget(m_sendButton);

    outerLayout->addWidget(footerContainer);

    applyStyle();
}

void DbcSendingSubView::applyStyle()
{
    const auto& colors = THEME.colors();

    m_outerScrollArea->setStyleSheet(
        QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_scrollContent->setStyleSheet(
        QString("QWidget#scrollContent { background-color: %1; }").arg(colors.surfaceMain.name()));
}

bool DbcSendingSubView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void DbcSendingSubView::populateFromModel(const SendingModel* model)
{
    if (!model)
    {
        return;
    }

    // Clear existing cards
    clearMessages();

    const Core::DbcConfig* dbc = model->currentDbcConfig();
    if (!dbc)
    {
        return;
    }

    // Create a card for each message in the DBC (View reads from Model)
    for (const auto& msgDef : dbc->messageDefinitions)
    {
        // Create message card
        auto* card =
            new Core::DbcMessageCard(QString::fromStdString(msgDef.messageName), msgDef.messageId,
                                     static_cast<int>(msgDef.signalDescriptions.size()), this);

        connect(card->headerCheckbox(), &Core::StyledCheckBox::toggled, this,
                [this, msgId = msgDef.messageId](const bool checked) {
                    emit messageSelectionChanged(msgId, checked);
                });

        // Add signal rows
        const uint16_t msgId = msgDef.messageId;
        for (const auto& sigDef : msgDef.signalDescriptions)
        {
            auto* signalRow = new Core::DbcSignalRowWidget(
                QString::fromStdString(sigDef.signalName), QString::fromStdString(sigDef.unit),
                sigDef.minimum, sigDef.maximum, card);

            QString signalName = QString::fromStdString(sigDef.signalName);

            // Connect signal selection checkbox
            if (auto* signalCheckbox = signalRow->selectionCheckbox())
            {
                signalCheckbox->setChecked(model->isSignalSelected(msgId, sigDef.signalName));
                connect(signalCheckbox, &QCheckBox::toggled, this,
                        [this, msgId, signalName](const bool checked) {
                            emit signalSelectionChanged(msgId, signalName, checked);
                        });
            }

            connect(signalRow->valueEditor(), &QLineEdit::textChanged, this,
                    [this, msgId, signalName](const QString& text) {
                        if (text.isEmpty())
                        {
                            return;
                        }
                        bool ok = false;
                        const double value = text.toDouble(&ok);
                        if (ok)
                        {
                            emit signalValueChanged(msgId, signalName, value);
                        }
                    });

            card->addSignalRow(signalRow);
        }

        card->updateHeaderFromSignals();

        // Add card to layout
        int insertIndex = m_cardsLayout->count() - 1;
        if (insertIndex < 0)
        {
            insertIndex = 0;
        }
        m_cardsLayout->insertWidget(insertIndex, card);
    }
}

void DbcSendingSubView::clearMessages() const
{
    if (!m_cardsLayout)
    {
        return;
    }
    while (m_cardsLayout->count() > 1)
    {
        const QLayoutItem* item = m_cardsLayout->takeAt(0);
        if (QWidget* widget = item->widget())
        {
            widget->blockSignals(true);
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }
}

}  // namespace Sending
