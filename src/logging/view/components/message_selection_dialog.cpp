/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "message_selection_dialog.hpp"

#include <QCheckBox>
#include <QScrollBar>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "logging/constants.hpp"
#include "logging/styles.hpp"
#include "logging/view/components/start_stop_button.hpp"

namespace Logging {

MessageSelectionDialog::MessageSelectionDialog(QWidget* parent) : QDialog(parent)
{
    setupUi();
}

void MessageSelectionDialog::setupUi()
{
    const auto& spacing = THEME.spacing();

    setWindowTitle(Constants::DIALOG_TITLE);
    setModal(true);
    setMinimumWidth(Constants::DIALOG_MIN_WIDTH);
    resize(Constants::DIALOG_MIN_WIDTH, Constants::DIALOG_START_HEIGHT);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                               spacing.spacingMd);
    layout->setSpacing(spacing.spacingMd);

    // Log type toggle
    m_buttonWidget = new QWidget(this);
    auto* buttonLayout = new QHBoxLayout(m_buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(spacing.spacingMd);

    m_rawLabel = new QLabel(Constants::DIALOG_RAW_LABEL, m_buttonWidget);
    m_rawLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_logTypeSwitch = new Core::StyledSwitch(m_buttonWidget);
    m_logTypeSwitch->setObjectName("logTypeSwitch");
    m_dbcLabel = new QLabel(Constants::DIALOG_DBC_LABEL, m_buttonWidget);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_rawLabel);
    buttonLayout->addWidget(m_logTypeSwitch);
    buttonLayout->addWidget(m_dbcLabel);
    buttonLayout->addStretch();
    layout->addWidget(m_buttonWidget);

    connect(m_logTypeSwitch, &Core::StyledSwitch::toggled, this,
            &MessageSelectionDialog::onLogTypeToggle);

    // Messages card
    m_messagesCard =
        new Core::CardWidget(Constants::DIALOG_MESSAGES_CARD_TITLE, QString(), QString(), this);
    if (auto* cardLayout = m_messagesCard->contentLayout())
    {
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

        // Placeholder lives inside the scroll area so the card title always stays at the top
        m_rawPlaceholder = new QLabel(Constants::DIALOG_RAW_PLACEHOLDER, m_scrollContent);
        m_rawPlaceholder->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_rawPlaceholder->setWordWrap(true);
        m_scrollLayout->addWidget(m_rawPlaceholder);
        m_scrollLayout->addStretch();

        m_scrollArea->setWidget(m_scrollContent);
        cardLayout->addWidget(m_scrollArea);
    }

    layout->addWidget(m_messagesCard, 1);

    // Start button
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addStretch();

    m_startBtn = new StartStopButton(bottomBar);
    m_startBtn->setRecordingState(false);
    connect(m_startBtn, &QPushButton::clicked, this, &MessageSelectionDialog::onStartClicked);
    bottomLayout->addWidget(m_startBtn);
    updateStartButton();

    layout->addWidget(bottomBar);

    applyStyle();
}

void MessageSelectionDialog::addMessageCard(Core::DbcMessageCard* card) const
{
    if (!card) return;
    m_scrollLayout->insertWidget(std::max(0, m_scrollLayout->count() - 1), card);
}

void MessageSelectionDialog::clearCards()
{
    m_messageCards.clear();
    m_signalRows.clear();

    // Index 0 is m_rawPlaceholder, last is the stretch — only remove cards in between
    while (m_scrollLayout->count() > 2)
    {
        QLayoutItem* item = m_scrollLayout->takeAt(1);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void MessageSelectionDialog::setDbcConfig(const Core::DbcConfig& config)
{
    clearCards();

    for (const auto& msgDef : config.messageDefinitions)
    {
        Core::DbcMessageCard::Config cardConfig;
        cardConfig.showCheckbox = true;
        cardConfig.startExpanded = false;
        cardConfig.checkboxTooltip = Constants::DIALOG_MESSAGE_CHECKBOX_TOOLTIP;

        auto* card = new Core::DbcMessageCard(
            QString::fromStdString(msgDef.messageName), msgDef.messageId,
            static_cast<int>(msgDef.signalDescriptions.size()), cardConfig, this);

        auto& rows = m_signalRows[msgDef.messageId];
        for (const auto& sigDef : msgDef.signalDescriptions)
        {
            Core::DbcSignalRowWidget::Config signalConfig;
            signalConfig.mode = Core::DbcSignalRowWidget::Mode::Selection;
            signalConfig.showSelectionCheckbox = true;
            signalConfig.showRange = false;

            QString name = QString::fromStdString(sigDef.signalName);
            auto* signalRow =
                new Core::DbcSignalRowWidget(name, QString::fromStdString(sigDef.unit),
                                             sigDef.minimum, sigDef.maximum, signalConfig, card);

            card->addSignalRow(signalRow);
            rows.emplace_back(name, signalRow);
            if (auto* cb = signalRow->selectionCheckbox())
                connect(cb, &QCheckBox::toggled, this, &MessageSelectionDialog::updateStartButton);
        }

        card->updateHeaderFromSignals();
        card->setVisible(m_logTypeSwitch->isChecked());
        if (auto* hcb = card->headerCheckbox())
            connect(hcb, &QCheckBox::clicked, this, &MessageSelectionDialog::updateStartButton);
        m_messageCards[msgDef.messageId] = card;
        m_scrollLayout->insertWidget(std::max(0, m_scrollLayout->count() - 1), card);
    }
    updateStartButton();
}

void MessageSelectionDialog::onStartClicked()
{
    std::map<uint32_t, QStringList> result;
    for (const auto& [msgId, rows] : m_signalRows)
    {
        QStringList names;
        for (const auto& [name, row] : rows)
        {
            if (const auto* cb = row->selectionCheckbox(); cb && cb->isChecked())
                names.append(name);
        }
        if (!names.isEmpty()) result[msgId] = std::move(names);
    }

    emit startRequested(m_logTypeSwitch->isChecked() ? DBC_BASED : RAW, result);
    accept();
}

void MessageSelectionDialog::onLogTypeToggle(const bool checked)
{
    m_rawPlaceholder->setVisible(!checked);
    for (const auto& card : m_messageCards | std::views::values) card->setVisible(checked);
    updateStartButton();
}

void MessageSelectionDialog::updateStartButton()
{
    if (!m_startBtn) return;
    if (!m_logTypeSwitch->isChecked())
    {
        m_startBtn->setEnabled(true);
        return;
    }
    bool anySelected = false;
    for (const auto& [msgId, rows] : m_signalRows)
    {
        for (const auto& [name, row] : rows)
        {
            if (const auto* cb = row->selectionCheckbox(); cb && cb->isChecked())
            {
                anySelected = true;
                break;
            }
        }
        if (anySelected) break;
    }
    m_startBtn->setEnabled(anySelected);
}

void MessageSelectionDialog::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    setStyleSheet(QString("QDialog { background-color: %1; }").arg(colors.surfaceMain.name()));

    if (m_scrollContent)
    {
        m_scrollContent->setStyleSheet(QString("QWidget#scrollContent { background-color: %1; }")
                                           .arg(colors.surfaceMain.name()));
    }
    if (m_rawPlaceholder)
    {
        m_rawPlaceholder->setStyleSheet(QString("QLabel { color: %1; padding-top: %2px; }")
                                            .arg(colors.textSecondary.name())
                                            .arg(spacing.spacingLg));
    }
    if (m_scrollArea && m_scrollArea->verticalScrollBar())
    {
        m_scrollArea->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());
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
}

auto MessageSelectionDialog::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QDialog::event(event);
}

}  // namespace Logging