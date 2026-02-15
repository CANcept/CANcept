#include "signal_list.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"
#include "monitoring/styles.hpp"

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
    m_mainLayout = new QVBoxLayout(this);
    m_scrollArea = new QScrollArea(this);
    m_scrollContent = new QWidget(m_scrollArea);
    m_cardsLayout = new QVBoxLayout(m_scrollContent);

    m_scrollArea->setWidget(m_scrollContent);
    m_mainLayout->addWidget(m_scrollArea);

    applyStyle();
}

void SignalList::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    if (!m_scrollContent) return;
    m_scrollContent->setStyleSheet(QString("background: %1;").arg(colors.surfaceMain.name()));

    if (!m_mainLayout) return;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    if (!m_scrollArea) return;
    m_scrollArea->setStyleSheet(QString("background-color: %1;").arg(colors.surfaceMain.name()));
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    if (!m_cardsLayout) return;
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(spacing.spacingSm);
    m_cardsLayout->addStretch();

    // Apply vertical scrollbar style
    if (m_scrollArea->verticalScrollBar())
        m_scrollArea->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());
}

void SignalList::onDbcChange()
{
    populateDecodedFromModel();
}

auto SignalList::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void SignalList::populateDecodedFromModel()
{
    if (!m_model) return;

    clearMessages();

    const int messageCount = m_model->rowCount(QModelIndex());
    for (int messageRow = 0; messageRow < messageCount; ++messageRow)
    {
        const QModelIndex messageIndex = m_model->index(messageRow, 0, QModelIndex());
        if (!messageIndex.isValid()) continue;

        const QString messageName =
            m_model->data(messageIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();
        auto* messageCard = new Core::CardWidget(messageName, QString(), nullptr, this);

        auto* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(THEME.spacing().spacingSm);

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
        idLabel->setText(QString("0x%1").arg(idText));
        horizontalLayout->addWidget(idLabel);

        messageCard->contentLayout()->addLayout(horizontalLayout);

        m_signalLists.append(new QWidget(messageCard));
        auto* signalsLayout = new QVBoxLayout(m_signalLists.last());
        signalsLayout->setContentsMargins(0, 0, 0, 0);
        signalsLayout->setSpacing(THEME.spacing().spacingSm);

        const int signalCount = m_model->rowCount(messageIndex);
        for (int signalRow = 0; signalRow < signalCount; ++signalRow)
        {
            const QModelIndex signalIndex = m_model->index(signalRow, 0, messageIndex);
            if (!signalIndex.isValid()) continue;

            const QString signalName =
                m_model->data(signalIndex, MonitoringModel::MonitoringRoles::Role_Name).toString();

            auto* signalCard =
                new Core::CardWidget(signalName, QString(), nullptr, m_signalLists.last());

            if (auto* contentLayout = signalCard->contentLayout())
            {
                auto* horizontalLayout = new QHBoxLayout();
                horizontalLayout->setSpacing(THEME.spacing().spacingLg);
                horizontalLayout->setContentsMargins(0, 0, 0, 0);

                auto* signalCheckbox = new Core::StyledCheckBox(signalCard);
                horizontalLayout->addWidget(signalCheckbox);

                horizontalLayout->addStretch();

                m_signalValues.append(new QLabel(signalCard));
                m_signalValues.last()->setText(
                    QString("%1%2")
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

        if (auto* contentLayout = messageCard->contentLayout())
        {
            contentLayout->addWidget(m_signalLists.last());
        }

        int insertIndex = m_cardsLayout->count() - 1;
        if (insertIndex < 0) insertIndex = 0;
        m_cardsLayout->addWidget(messageCard);
    }

    LOG_INF("MonitoringComponent", "Signal List view built...");
}

void SignalList::clearMessages()
{
    if (!m_cardsLayout) return;

    while (QLayoutItem* item = m_cardsLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }
    while (!m_signalValues.empty())
    {
        if (m_signalValues.last() != nullptr)
        {
            delete m_signalValues.last();
        }
        m_signalValues.removeLast();
    }
    m_signalLists.clear();
}

void SignalList::updateViewData()
{
    if (!m_model) return;

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
                    QString("%1%2")
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
