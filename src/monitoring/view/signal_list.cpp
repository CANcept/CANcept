#include "signal_list.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

SignalList::SignalList(QWidget* parent)
    : QWidget(parent),
      m_model(nullptr),
      m_scrollArea(nullptr),
      m_scrollContent(nullptr),
      m_cardsLayout(nullptr)
{
    setupUi();
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

void SignalList::setModel(MonitoringModel* model)
{
    m_model = model;
    if (!m_model) return;

    // Connect model signals
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &SignalList::onModelRowsInserted);
    connect(m_model, &QAbstractItemModel::dataChanged, this, &SignalList::onModelDataChanged);

    // Populate initial data
    populateFromModel();
}

void SignalList::populateFromModel()
{
    if (!m_model) return;

    clearMessages();

    // Iterate through all frames (top-level items)
    const int frameCount = m_model->rowCount(QModelIndex());
    for (int frameRow = 0; frameRow < frameCount; ++frameRow)
    {
        const QModelIndex frameIndex = m_model->index(frameRow, 0, QModelIndex());
        if (!frameIndex.isValid()) continue;

        // Create message card with frame display data
        const QString frameDisplay = m_model->data(frameIndex, Qt::DisplayRole).toString();
        auto* messageCard =
            new Core::CardWidget(Constants::MESSAGES_LABEL, QString(), nullptr, this);

        // Create expand button
        auto* expandButton = new QPushButton(m_scrollContent);
        expandButton->setText("▼");
        expandButton->setMaximumWidth(40);
        expandButton->setCheckable(true);
        expandButton->setChecked(false);

        messageCard->contentLayout()->addWidget(expandButton);

        // Create checkbox for message selection
        auto* messageCheckbox = new Core::StyledCheckBox(messageCard);
        const Qt::CheckState messageCheckState =
            static_cast<Qt::CheckState>(m_model->data(frameIndex, Qt::CheckStateRole).toInt());
        messageCheckbox->setCheckState(messageCheckState);

        messageCard->contentLayout()->insertWidget(0, messageCheckbox);

        connect(messageCheckbox, &QCheckBox::toggled, this, [this, frameIndex](const bool checked) {
            m_model->setData(frameIndex, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        });

        // Create signal cards container (initially hidden)
        auto* signalsContainer = new QWidget(messageCard);
        auto* signalsLayout = new QVBoxLayout(signalsContainer);
        signalsLayout->setContentsMargins(0, 0, 0, 0);
        signalsLayout->setSpacing(THEME.spacing().spacingSm);

        // Populate signals for this message
        const int signalCount = m_model->rowCount(frameIndex);
        for (int signalRow = 0; signalRow < signalCount; ++signalRow)
        {
            const QModelIndex signalIndex = m_model->index(signalRow, 0, frameIndex);
            if (!signalIndex.isValid()) continue;

            const QString signalDisplay = m_model->data(signalIndex, Qt::DisplayRole).toString();
            const Qt::CheckState signalCheckState =
                static_cast<Qt::CheckState>(m_model->data(signalIndex, Qt::CheckStateRole).toInt());

            // Create signal card
            auto* signalCard =
                new Core::CardWidget(signalDisplay, QString(), nullptr, signalsContainer);
            signalCard->se

            // Add value display/editor in signal card content
            if (auto* contentLayout = signalCard->contentLayout())
            {
                auto* valueLabel = new QLabel(signalCard);
                valueLabel->setText("Current Value: ");
                contentLayout->addWidget(valueLabel);

                // Add checkbox for signal selection
                auto* signalCheckbox = new Core::StyledCheckBox(signalCard);
                signalCheckbox->setCheckState(signalCheckState);
                contentLayout->addWidget(signalCheckbox);

                connect(signalCheckbox, &QCheckBox::toggled, this,
                        [this, signalIndex](const bool checked) -> void {
                            m_model->setData(signalIndex, checked ? Qt::Checked : Qt::Unchecked,
                                             Qt::CheckStateRole);
                        });
            }

            signalsLayout->addWidget(signalCard);
        }

        signalsContainer->hide();

        // Connect expand button to show/hide signals
        connect(expandButton, &QPushButton::toggled, signalsContainer, &QWidget::setVisible);

        // Add signals container to message card
        if (auto* contentLayout = messageCard->contentLayout())
        {
            contentLayout->addWidget(signalsContainer);
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

void SignalList::onModelRowsInserted(const QModelIndex& parent, int first, int last)
{
    // Rebuild the entire view when new rows are inserted
    populateFromModel();
}

void SignalList::onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    // Update checkboxes when model data changes
    if (!m_model) return;

    // For now, rebuild the view. In a more optimized version,
    // you could update only the affected items.
    populateFromModel();
}

}  // namespace Monitoring