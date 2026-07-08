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

#include "manipulation_dialog.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QSignalBlocker>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/common/styled_checkbox.hpp"
#include "core/widgets/common/styled_line_edit.hpp"
#include "core/widgets/dbc_message_card.hpp"
#include "core/widgets/dbc_signal_row.hpp"
#include "manipulation/constants.hpp"
#include "manipulation/styles.hpp"
#include "math/service/variable_registry.hpp"
#include "providers/manipulation_row_type_providers.hpp"

namespace Manipulation {

ManipulationDialog::ManipulationDialog(QWidget* parent)
    : QDialog(parent), m_model(std::make_unique<ManipulationDialogModel>(this))
{
    setupUI();
}

auto ManipulationDialog::open(const bool isRaw) -> int
{
    m_titleLabel->setText(isRaw ? Constants::MANIPULATION_DIALOG_RAW_TITLE
                                : Constants::MANIPULATION_DIALOG_DBC_TITLE);
    m_model->open(isRaw);
    rebuildStrategyRow(isRaw);
    populateInsertMessageCombo();
    return exec();
}

auto ManipulationDialog::acquire() const -> std::optional<ManipulationEntry>
{
    for (const auto& [signalName, row] : m_insertSignalRows)
    {
        const auto* checkbox = row->selectionCheckbox();
        if (auto* edit = row->valueLineEdit(); edit && (!checkbox || checkbox->isChecked()))
        {
            m_model->setInsertSignalValue(signalName, edit->text().toDouble());
        }
    }
    return m_model->acquire();
}

void ManipulationDialog::setVariableRegistry(Math::VariableRegistry* registry)
{
    m_variableRegistry = registry;
}

auto ManipulationDialog::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QDialog::event(event);
}

static auto createSectionLayout(Core::CardWidget* card, QPushButton*& addButton,
                                QVBoxLayout*& rowsLayout, QScrollArea*& scrollArea,
                                const QString& addLabel) -> void
{
    const auto& spacing = THEME.spacing();

    card->setMinimumHeight(Constants::SECTION_MIN_HEIGHT);
    auto* content = card->contentLayout();

    auto* scrollContent = new QWidget(card);
    rowsLayout = new QVBoxLayout(scrollContent);
    rowsLayout->setContentsMargins(0, 0, 0, 0);
    rowsLayout->setSpacing(spacing.spacingXs);
    rowsLayout->addStretch();

    scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(scrollContent);

    if (auto* vp = scrollArea->viewport())
    {
        scrollArea->setAttribute(Qt::WA_TranslucentBackground);
        vp->setAutoFillBackground(false);
        vp->setAttribute(Qt::WA_TranslucentBackground);
    }

    content->addWidget(scrollArea);

    addButton = new QPushButton(addLabel, card);
    content->addWidget(addButton);
}

void ManipulationDialog::setupUI()
{
    const auto& spacing = THEME.spacing();

    setWindowTitle(Constants::MANIPULATION_WINDOW_NAME);
    setObjectName(Constants::MANIPULATION_DIALOG_OBJECT_NAME);
    setMinimumWidth(Constants::DIALOG_MIN_WIDTH);
    setModal(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                 spacing.spacingMd);
    m_layout->setSpacing(spacing.spacingMd);

    m_titleLabel = new QLabel(Constants::MANIPULATION_DIALOG_RAW_TITLE, this);
    m_layout->addWidget(m_titleLabel);

    // Trigger card
    m_triggerCard = new Core::CardWidget(Constants::MANIPULATION_DIALOG_TRIGGER_LABEL, QString(),
                                         QString(), this);
    createSectionLayout(m_triggerCard, m_addTriggerButton, m_triggerRowsLayout, m_triggerScrollArea,
                        "+ Add");
    connect(m_addTriggerButton, &QPushButton::clicked, this,
            [this]() -> void { m_model->addTrigger(); });
    m_layout->addWidget(m_triggerCard);

    // Strategy section
    m_strategyLabel = new QLabel(Constants::MANIPULATION_DIALOG_STRATEGY_LABEL, this);
    m_layout->addWidget(m_strategyLabel);

    // Effect card
    m_effectCard = new Core::CardWidget(Constants::MANIPULATION_DIALOG_EFFECT_LABEL, QString(),
                                        QString(), this);
    createSectionLayout(m_effectCard, m_addEffectButton, m_effectRowsLayout, m_effectScrollArea,
                        "+ Add");
    connect(m_addEffectButton, &QPushButton::clicked, this,
            [this]() -> void { m_model->addEffect(); });
    m_layout->addWidget(m_effectCard);
    m_effectCard->setVisible(false);

    // Insert card
    m_insertCard = new Core::CardWidget(Constants::MANIPULATION_DIALOG_INSERT_LABEL, QString(),
                                        QString(), this);
    auto* insertContent = m_insertCard->contentLayout();

    auto* messageRow = new QHBoxLayout();
    messageRow->setContentsMargins(0, 0, 0, 0);
    messageRow->setSpacing(spacing.spacingSm);
    auto* messageLabel = new QLabel("Message:", m_insertCard);
    m_insertMessageCombo = new Core::StyledComboBox(m_insertCard);
    messageRow->addWidget(messageLabel);
    messageRow->addWidget(m_insertMessageCombo, 1);
    insertContent->addLayout(messageRow);

    m_insertMessageContainer = new QWidget(m_insertCard);
    m_insertMessageContainerLayout = new QVBoxLayout(m_insertMessageContainer);
    m_insertMessageContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_insertMessageContainerLayout->setSpacing(spacing.spacingXs);

    m_insertCurrentMessageHint = new QLabel(
        Constants::MANIPULATION_DIALOG_INSERT_CURRENT_MESSAGE_HINT, m_insertMessageContainer);
    m_insertCurrentMessageHint->setWordWrap(true);
    m_insertMessageContainerLayout->addWidget(m_insertCurrentMessageHint);
    m_insertCurrentMessageHint->setVisible(false);

    m_insertMessageScrollArea = new QScrollArea(m_insertCard);
    m_insertMessageScrollArea->setWidgetResizable(true);
    m_insertMessageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_insertMessageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_insertMessageScrollArea->setFrameShape(QFrame::NoFrame);
    m_insertMessageScrollArea->setWidget(m_insertMessageContainer);
    m_insertMessageScrollArea->setMaximumHeight(Constants::SECTION_MIN_HEIGHT);

    if (auto* vp = m_insertMessageScrollArea->viewport())
    {
        m_insertMessageScrollArea->setAttribute(Qt::WA_TranslucentBackground);
        vp->setAutoFillBackground(false);
        vp->setAttribute(Qt::WA_TranslucentBackground);
    }

    insertContent->addWidget(m_insertMessageScrollArea);

    connect(m_insertMessageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ManipulationDialog::onInsertMessageChanged);

    m_insertCard->setMinimumHeight(Constants::SECTION_MIN_HEIGHT);
    m_layout->addWidget(m_insertCard);
    m_insertCard->setVisible(false);

    // Mutation section (single fixed row)
    m_mutationLabel = new QLabel(Constants::MANIPULATION_DIALOG_MUTATION_LABEL, this);
    m_layout->addWidget(m_mutationLabel);

    m_mutationRow = new ManipulationRowWidget(getMutationProviders(), /*showRemove=*/false, this);
    connect(m_mutationRow, &ManipulationRowWidget::changed, this,
            [this]() -> void { m_model->setMutation(m_mutationRow->currentEntry()); });
    m_model->setMutation(m_mutationRow->currentEntry());
    m_layout->addWidget(m_mutationRow);

    m_layout->addStretch();

    // Bottom bar
    auto* bottomBar = new QWidget(this);
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addStretch();

    m_confirmButton = new QPushButton("Confirm", bottomBar);
    connect(m_confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(m_confirmButton);

    m_layout->addWidget(bottomBar);

    // Connect model signals
    connect(m_model.get(), &ManipulationDialogModel::triggerAdded, this,
            &ManipulationDialog::onTriggerAdded);
    connect(m_model.get(), &ManipulationDialogModel::triggerRemoved, this,
            &ManipulationDialog::onTriggerRemoved);
    connect(m_model.get(), &ManipulationDialogModel::effectAdded, this,
            &ManipulationDialog::onEffectAdded);
    connect(m_model.get(), &ManipulationDialogModel::effectRemoved, this,
            &ManipulationDialog::onEffectRemoved);

    applyStyle();
}

void ManipulationDialog::onTriggerAdded(const int index)
{
    auto* row = new ManipulationRowWidget(
        m_model->isRaw() ? getRawTriggerProviders() : getDbcTriggerProviders(dbcConfig()),
        /*showRemove=*/true, m_triggerRowsLayout->parentWidget());

    row->setFromEntry(m_model->triggerEntries().at(index));

    connect(row, &ManipulationRowWidget::changed, this, [this, row]() -> void {
        const int idx = m_triggerRows.indexOf(row);
        if (idx >= 0)
        {
            m_model->setTrigger(idx, row->currentEntry());
        }
    });

    connect(row, &ManipulationRowWidget::removeRequested, this, [this, row]() -> void {
        const int idx = m_triggerRows.indexOf(row);
        if (idx >= 0)
        {
            m_model->removeTrigger(idx);
        }
    });

    m_model->setTrigger(index, row->currentEntry());

    const int insertIdx = m_triggerRowsLayout->count() - 1;
    m_triggerRowsLayout->insertWidget(insertIdx, row);
    m_triggerRows.append(row);
}

void ManipulationDialog::onTriggerRemoved(const int index)
{
    if (index < 0 || index >= m_triggerRows.size())
    {
        return;
    }
    auto* row = m_triggerRows.takeAt(index);
    m_triggerRowsLayout->removeWidget(row);
    row->deleteLater();
}

void ManipulationDialog::onEffectAdded(const int index)
{
    auto* row = new ManipulationRowWidget(
        m_model->isRaw() ? getRawEffectProviders() : getDbcEffectProviders(dbcConfig()),
        /*showRemove=*/true, m_effectRowsLayout->parentWidget());

    row->setFromEntry(m_model->effectEntries().at(index));

    connect(row, &ManipulationRowWidget::changed, this, [this, row]() -> void {
        if (const int idx = m_effectRows.indexOf(row); idx >= 0)
        {
            m_model->setEffect(idx, row->currentEntry());
        }
    });

    connect(row, &ManipulationRowWidget::removeRequested, this, [this, row]() -> void {
        if (const int idx = m_effectRows.indexOf(row); idx >= 0)
        {
            m_model->removeEffect(idx);
        }
    });

    m_model->setEffect(index, row->currentEntry());

    const int insertIdx = m_effectRowsLayout->count() - 1;
    m_effectRowsLayout->insertWidget(insertIdx, row);
    m_effectRows.append(row);
}

void ManipulationDialog::onEffectRemoved(const int index)
{
    if (index < 0 || index >= m_effectRows.size())
    {
        return;
    }
    auto* row = m_effectRows.takeAt(index);
    m_effectRowsLayout->removeWidget(row);
    row->deleteLater();
}

void ManipulationDialog::updateStrategySubAreaVisibility()
{
    const int typeIndex = m_strategyRow->currentEntry().typeIndex;
    m_effectCard->setVisible(typeIndex == 0);
    m_insertCard->setVisible(!m_model->isRaw() && typeIndex == 3);
    m_layout->activate();
    update();
}

void ManipulationDialog::rebuildStrategyRow(const bool isRaw)
{
    if (m_strategyRow)
    {
        m_layout->removeWidget(m_strategyRow);
        m_strategyRow->setVisible(false);
        m_strategyRow->deleteLater();
        m_strategyRow = nullptr;
    }

    m_strategyRow = new ManipulationRowWidget(
        isRaw ? getRawStrategyProviders() : getDbcStrategyProviders(), /*showRemove=*/false, this);
    connect(m_strategyRow, &ManipulationRowWidget::changed, this, [this]() -> void {
        m_model->setStrategy(m_strategyRow->currentEntry());
        updateStrategySubAreaVisibility();
    });
    m_model->setStrategy(m_strategyRow->currentEntry());

    const int labelIndex = m_layout->indexOf(m_strategyLabel);
    m_layout->insertWidget(labelIndex + 1, m_strategyRow);

    if (!m_dialogHeightLocked)
    {
        m_dialogHeightLocked = true;
        m_effectCard->setVisible(true);
        m_layout->activate();
        setFixedHeight(sizeHint().height());
    }

    updateStrategySubAreaVisibility();
}

void ManipulationDialog::populateInsertMessageCombo()
{
    const QSignalBlocker blocker(m_insertMessageCombo);
    m_insertMessageCombo->clear();

    m_insertMessageCombo->addItem(Constants::MANIPULATION_DIALOG_INSERT_CURRENT_MESSAGE_LABEL);

    if (const auto* config = dbcConfig())
    {
        for (const auto& msg : config->messageDefinitions)
        {
            m_insertMessageCombo->addItem(QString::fromStdString(msg.messageName), msg.messageId);
        }
    }

    m_insertMessageCombo->setCurrentIndex(0);
    onInsertMessageChanged(0);
}

void ManipulationDialog::onInsertMessageChanged(const int index)
{
    if (m_insertMessageCard)
    {
        m_insertMessageContainerLayout->removeWidget(m_insertMessageCard);
        m_insertMessageCard->setVisible(false);
        m_insertMessageCard->deleteLater();
        m_insertMessageCard = nullptr;
    }
    m_insertSignalRows.clear();

    if (index <= 0)
    {
        if (index == 0)
        {
            m_model->setInsertUseCurrentMessage();
        }
        m_insertCurrentMessageHint->setVisible(index == 0);
        m_insertMessageContainerLayout->activate();
        m_insertCard->update();
        return;
    }
    m_insertCurrentMessageHint->setVisible(false);

    const auto* config = dbcConfig();
    if (!config)
    {
        return;
    }

    const auto messageId = static_cast<uint32_t>(m_insertMessageCombo->itemData(index).toUInt());
    const Core::DbcMessageDescription* msgDef = nullptr;
    for (const auto& msg : config->messageDefinitions)
    {
        if (msg.messageId == messageId)
        {
            msgDef = &msg;
            break;
        }
    }
    if (!msgDef)
    {
        return;
    }

    m_model->setInsertMessage(messageId, *msgDef);

    Core::DbcMessageCard::Config cardConfig;
    cardConfig.showCheckbox = false;
    cardConfig.startExpanded = true;
    m_insertMessageCard = new Core::DbcMessageCard(
        QString::fromStdString(msgDef->messageName), messageId,
        static_cast<int>(msgDef->signalDescriptions.size()), cardConfig, m_insertMessageContainer);

    for (const auto& sig : msgDef->signalDescriptions)
    {
        Core::DbcSignalRowWidget::Config rowConfig;
        rowConfig.mode = Core::DbcSignalRowWidget::Mode::PlainValue;
        rowConfig.showRange = true;
        auto* row = new Core::DbcSignalRowWidget(QString::fromStdString(sig.signalName),
                                                 QString::fromStdString(sig.unit), sig.minimum,
                                                 sig.maximum, rowConfig, m_insertMessageCard);

        m_insertSignalRows.emplace_back(sig.signalName, row);
        m_insertMessageCard->addSignalRow(row);
    }

    m_insertMessageContainerLayout->addWidget(m_insertMessageCard);
    m_insertMessageContainerLayout->activate();
    m_insertCard->update();
}

auto ManipulationDialog::dbcConfig() const -> const Core::DbcConfig*
{
    return m_variableRegistry ? m_variableRegistry->dbcConfig() : nullptr;
}

void ManipulationDialog::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    QPalette pal = palette();
    pal.setColor(QPalette::Window, colors.surfaceMain);
    setPalette(pal);

    QPixmap pixmap(Constants::MANIPULATION_CONFIRM_ICON_PATH);
    if (!pixmap.isNull())
    {
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), colors.textPrimary);
        painter.end();
        m_confirmButton->setIcon(QIcon(pixmap));
        m_confirmButton->setIconSize(QSize(spacing.spacingMd, spacing.spacingMd));
    }

    setStyleSheet(QString(R"(
        #manipulationDialog {
            background-color: %1;
            color: %2;
        }
        #manipulationDialog QPushButton {
            background-color: %3;
            color: %2;
            border: none;
            border-radius: %4px;
            padding: %5px %6px;
            font-weight: %7;
            font-size: %8px;
            text-align: center;
        }
        #manipulationDialog QPushButton:hover {
            background-color: %9;
        }
        #manipulationDialog QPushButton:pressed {
            background-color: %9;
        }
        #manipulationDialog QScrollArea,
        #manipulationDialog QScrollArea > QWidget {
            background: transparent;
            border: none;
        }
    )")
                      .arg(colors.surfaceMain.name(QColor::HexArgb))
                      .arg(colors.textPrimary.name())
                      .arg(colors.colorPrimary.name())
                      .arg(spacing.radiusSm)
                      .arg(spacing.spacingSm)
                      .arg(spacing.spacingMd)
                      .arg(spacing.fontWeightMedium)
                      .arg(spacing.fontSizeSm)
                      .arg(colors.colorPrimaryHover.name()));

    if (m_titleLabel)
    {
        m_titleLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: %3;")
                                        .arg(colors.textPrimary.name())
                                        .arg(spacing.fontSizeMd)
                                        .arg(spacing.fontWeightNormal));
    }

    const QString sectionLabelStyle = QString("color: %1; font-size: %2px; font-weight: %3;")
                                          .arg(colors.textSecondary.name())
                                          .arg(spacing.fontSizeSm)
                                          .arg(spacing.fontWeightMedium);

    if (m_strategyLabel)
    {
        m_strategyLabel->setStyleSheet(sectionLabelStyle);
    }

    if (m_mutationLabel)
    {
        m_mutationLabel->setStyleSheet(sectionLabelStyle);
    }

    const QString scrollBarStyle = Style::Common::verticalScrollBar();
    if (m_triggerScrollArea)
    {
        m_triggerScrollArea->setStyleSheet(scrollBarStyle);
    }
    if (m_effectScrollArea)
    {
        m_effectScrollArea->setStyleSheet(scrollBarStyle);
    }
    if (m_insertMessageScrollArea)
    {
        m_insertMessageScrollArea->setStyleSheet(scrollBarStyle);
    }
}

}  // namespace Manipulation