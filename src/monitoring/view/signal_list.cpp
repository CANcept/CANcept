#include "signal_list.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

SignalList::SignalList(QWidget* parent, MonitoringModel* model)
    : QWidget(parent),
      m_model(model),
      m_scrollArea(nullptr),
      m_scrollContent(nullptr),
      m_cardsLayout(nullptr)
{
    setupUi();

    populateDecodedFromModel();
}

void SignalList::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create scroll area for message cards
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollContent = new QWidget(m_scrollArea);
    m_scrollContent->setObjectName("scrollContent");
    m_scrollContent->setStyleSheet(
        QString("QWidget#scrollContent { background-color: %1; }").arg(colors.surfaceMain.name()));

    m_cardsLayout = new QVBoxLayout(m_scrollContent);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(spacing.spacingSm);
    m_cardsLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);
}

void SignalList::populateDecodedFromModel()
{
    if (!m_model) return;

    clearMessages();

    // Iterate through all frames (top-level items)
    const int messageCount = m_model->rowCount(QModelIndex());
    for (int messageRow = 0; messageRow < messageCount; ++messageRow)
    {
        const QModelIndex messageIndex = m_model->index(messageRow, 0, QModelIndex());
        if (!messageIndex.isValid()) continue;

        // Create message card with frame display data
        const QString messageName =
            m_model->data(messageIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();
        auto* messageCard = new Core::CardWidget(messageName, QString(), nullptr, this);

        auto* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(THEME.spacing().spacingSm);

        // Create expand button
        auto* expandButton = new QPushButton(m_scrollContent);
        expandButton->setIcon(QIcon(Constants::ARROW_RIGHT_BUTTON_ICON_PATH));
        expandButton->setStyleSheet("QPushButton { border: none; background-color: transparent; }");
        expandButton->setMaximumWidth(40);
        expandButton->setCheckable(true);
        expandButton->setChecked(false);
        horizontalLayout->addWidget(expandButton);

        const QString idText =
            m_model->data(messageIndex, MonitoringModel::MonitoringRoles::Role_ID).toString();
        auto* idLabel = new QLabel(messageCard);
        idLabel->setText(idText);
        horizontalLayout->addWidget(idLabel);

        messageCard->contentLayout()->addLayout(horizontalLayout);

        // Create signal cards container (initially hidden)
        m_signalLists.append(new QWidget(messageCard));
        auto* signalsLayout = new QVBoxLayout(m_signalLists.last());
        signalsLayout->setContentsMargins(0, 0, 0, 0);
        signalsLayout->setSpacing(THEME.spacing().spacingSm);

        // Populate signals for this message
        const int signalCount = m_model->rowCount(messageIndex);
        for (int signalRow = 0; signalRow < signalCount; ++signalRow)
        {
            const QModelIndex signalIndex = m_model->index(signalRow, 0, messageIndex);
            if (!signalIndex.isValid()) continue;

            const QString signalName =
                m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();

            // Create signal card
            auto* signalCard =
                new Core::CardWidget(signalName, QString(), nullptr, m_signalLists.last());

            // Add value display/editor in signal card content
            if (auto* contentLayout = signalCard->contentLayout())
            {
                auto* horizontalLayout = new QHBoxLayout();
                horizontalLayout->setSpacing(THEME.spacing().spacingLg);
                horizontalLayout->setContentsMargins(0, 0, 0,
                                                     0);  // Optional: tighten up the card look

                // 1. Checkbox (Far Left)
                auto* signalCheckbox = new Core::StyledCheckBox(signalCard);
                horizontalLayout->addWidget(signalCheckbox);

                // 2. THE "SPRING" (Pushes everything after this to the right)
                horizontalLayout->addStretch();

                // 3. Value Label (Aligned to the Right)
                m_signalValues.append(new QLabel(signalCard));
                m_signalValues.last()->setText(
                    QString("%1 / %2")
                        .arg(m_model
                                 ->data(signalIndex,
                                        MonitoringModel::MonitoringRoles::Role_LatestValue)
                                 .toString())
                        .arg(m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Unit)
                                 .toString()));
                m_signalValues.last()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                horizontalLayout->addWidget(m_signalValues.last());

                contentLayout->addLayout(horizontalLayout);

                connect(signalCheckbox, &Core::StyledCheckBox::toggled, this,
                        [this, idText, signalName](bool checked) -> void {
                            emit signalMonitoringToggled(checked, idText, signalName);
                        });
            }

            signalsLayout->addWidget(signalCard);
        }

        m_signalLists.last()->hide();

        // Connect expand button to show/hide signals
        connect(expandButton, &QPushButton::toggled, m_signalLists.last(), &QWidget::setVisible);

        connect(expandButton, &QPushButton::toggled, this,
                [expandButton](const bool expanded) -> void {
                    if (expanded)
                    {
                        expandButton->setIcon(QIcon(Constants::ARROW_DOWN_BUTTON_ICON_PATH));
                    } else
                    {
                        expandButton->setIcon(QIcon(Constants::ARROW_RIGHT_BUTTON_ICON_PATH));
                    }
                });

        // Add signals container to message card
        if (auto* contentLayout = messageCard->contentLayout())
        {
            contentLayout->addWidget(m_signalLists.last());
        }

        // Add message card to main layout
        int insertIndex = m_cardsLayout->count() - 1;
        if (insertIndex < 0) insertIndex = 0;
        m_cardsLayout->insertWidget(insertIndex, messageCard);
    }
}

void SignalList::clearMessages()
{
    if (!m_cardsLayout) return;

    while (QLayoutItem* item = m_cardsLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();  // Deletes the widget safely
        }
        delete item;  // Deletes the layout spacer or container
    }
}

void SignalList::updateViewData()
{
    if (!m_model) return;

    // Iterate through all frames (top-level items)
    int absoluteSignalId = 0;
    const int messageCount = m_model->rowCount(QModelIndex());
    for (int messageRow = 0; messageRow < messageCount; ++messageRow)
    {
        const QModelIndex messageIndex = m_model->index(messageRow, 0, QModelIndex());

        const int signalCount = m_model->rowCount(messageIndex);
        for (int signalRow = 0; signalRow < signalCount; ++signalRow)
        {
            const QModelIndex signalIndex = m_model->index(signalRow, 0, messageIndex);

            m_signalValues.at(absoluteSignalId)
                ->setText(
                    QString("%1 / %2")
                        .arg(m_model
                                 ->data(signalIndex,
                                        MonitoringModel::MonitoringRoles::Role_LatestValue)
                                 .toString())
                        .arg(m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Unit)
                                 .toString()));
            absoluteSignalId++;
        }
    }
}

}  // namespace Monitoring
