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

#include "fault_injector_dialog.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "fault_injector/constants.hpp"
#include "fault_injector/styles.hpp"
#include "providers/fault_row_type_providers.hpp"

namespace FaultInjector {

FaultInjectorDialog::FaultInjectorDialog(QWidget* parent)
    : QDialog(parent), m_model(std::make_unique<FaultInjectorDialogModel>(this))
{
    setupUI();
}

auto FaultInjectorDialog::open(const bool isRaw) -> int
{
    m_titleLabel->setText(isRaw ? Constants::FAULT_INJECTOR_DIALOG_RAW_TITLE
                                : Constants::FAULT_INJECTOR_DIALOG_DBC_TITLE);
    m_model->open(isRaw);
    return exec();
}

auto FaultInjectorDialog::acquire() const -> std::optional<Fault>
{
    return m_model->acquire();
}

auto FaultInjectorDialog::event(QEvent* event) -> bool
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

void FaultInjectorDialog::setupUI()
{
    const auto& spacing = THEME.spacing();

    setWindowTitle(Constants::FAULT_INJECTOR_WINDOW_NAME);
    setObjectName(Constants::FAULT_INJECTOR_DIALOG_OBJECT_NAME);
    setMinimumWidth(Constants::DIALOG_MIN_WIDTH);
    setModal(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                 spacing.spacingMd);
    m_layout->setSpacing(spacing.spacingMd);

    m_titleLabel = new QLabel(Constants::FAULT_INJECTOR_DIALOG_RAW_TITLE, this);
    m_layout->addWidget(m_titleLabel);

    // Trigger card
    m_triggerCard = new Core::CardWidget(Constants::FAULT_INJECTOR_DIALOG_TRIGGER_LABEL, QString(),
                                         QString(), this);
    createSectionLayout(m_triggerCard, m_addTriggerButton, m_triggerRowsLayout, m_triggerScrollArea,
                        "+ Add");
    connect(m_addTriggerButton, &QPushButton::clicked, this,
            [this]() -> void { m_model->addTrigger(); });
    m_layout->addWidget(m_triggerCard);

    // Effect card
    m_effectCard = new Core::CardWidget(Constants::FAULT_INJECTOR_DIALOG_EFFECT_LABEL, QString(),
                                        QString(), this);
    createSectionLayout(m_effectCard, m_addEffectButton, m_effectRowsLayout, m_effectScrollArea,
                        "+ Add");
    connect(m_addEffectButton, &QPushButton::clicked, this,
            [this]() -> void { m_model->addEffect(); });
    m_layout->addWidget(m_effectCard);

    // Strategy section (single fixed row)
    m_strategyLabel = new QLabel(Constants::FAULT_INJECTOR_DIALOG_STRATEGY_LABEL, this);
    m_layout->addWidget(m_strategyLabel);

    m_strategyRow = new FaultRowWidget(getStrategyProviders(), /*showRemove=*/false, this);
    connect(m_strategyRow, &FaultRowWidget::changed, this,
            [this]() -> void { m_model->setStrategy(m_strategyRow->currentEntry()); });
    m_layout->addWidget(m_strategyRow);

    // Mutation section (single fixed row)
    m_mutationLabel = new QLabel(Constants::FAULT_INJECTOR_DIALOG_MUTATION_LABEL, this);
    m_layout->addWidget(m_mutationLabel);

    m_mutationRow = new FaultRowWidget(getMutationProviders(), /*showRemove=*/false, this);
    connect(m_mutationRow, &FaultRowWidget::changed, this,
            [this]() -> void { m_model->setMutation(m_mutationRow->currentEntry()); });
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
    connect(m_model.get(), &FaultInjectorDialogModel::triggerAdded, this,
            &FaultInjectorDialog::onTriggerAdded);
    connect(m_model.get(), &FaultInjectorDialogModel::triggerRemoved, this,
            &FaultInjectorDialog::onTriggerRemoved);
    connect(m_model.get(), &FaultInjectorDialogModel::effectAdded, this,
            &FaultInjectorDialog::onEffectAdded);
    connect(m_model.get(), &FaultInjectorDialogModel::effectRemoved, this,
            &FaultInjectorDialog::onEffectRemoved);

    applyStyle();
}

void FaultInjectorDialog::onTriggerAdded(const int index)
{
    auto* row =
        new FaultRowWidget(m_model->isRaw() ? getRawTriggerProviders() : getDbcTriggerProviders(),
                           /*showRemove=*/true, m_triggerRowsLayout->parentWidget());

    row->setFromEntry(m_model->triggerEntries().at(index));

    connect(row, &FaultRowWidget::changed, this, [this, row]() -> void {
        const int idx = m_triggerRows.indexOf(row);
        if (idx >= 0)
        {
            m_model->setTrigger(idx, row->currentEntry());
        }
    });

    connect(row, &FaultRowWidget::removeRequested, this, [this, row]() -> void {
        const int idx = m_triggerRows.indexOf(row);
        if (idx >= 0)
        {
            m_model->removeTrigger(idx);
        }
    });

    const int insertIdx = m_triggerRowsLayout->count() - 1;  // before stretch
    m_triggerRowsLayout->insertWidget(insertIdx, row);
    m_triggerRows.append(row);
}

void FaultInjectorDialog::onTriggerRemoved(const int index)
{
    if (index < 0 || index >= m_triggerRows.size())
    {
        return;
    }
    auto* row = m_triggerRows.takeAt(index);
    m_triggerRowsLayout->removeWidget(row);
    row->deleteLater();
}

void FaultInjectorDialog::onEffectAdded(const int index)
{
    auto* row =
        new FaultRowWidget(m_model->isRaw() ? getRawEffectProviders() : getDbcEffectProviders(),
                           /*showRemove=*/true, m_effectRowsLayout->parentWidget());

    row->setFromEntry(m_model->effectEntries().at(index));

    connect(row, &FaultRowWidget::changed, this, [this, row]() -> void {
        if (const int idx = m_effectRows.indexOf(row); idx >= 0)
        {
            m_model->setEffect(idx, row->currentEntry());
        }
    });

    connect(row, &FaultRowWidget::removeRequested, this, [this, row]() -> void {
        if (const int idx = m_effectRows.indexOf(row); idx >= 0)
        {
            m_model->removeEffect(idx);
        }
    });

    const int insertIdx = m_effectRowsLayout->count() - 1;
    m_effectRowsLayout->insertWidget(insertIdx, row);
    m_effectRows.append(row);
}

void FaultInjectorDialog::onEffectRemoved(const int index)
{
    if (index < 0 || index >= m_effectRows.size())
    {
        return;
    }
    auto* row = m_effectRows.takeAt(index);
    m_effectRowsLayout->removeWidget(row);
    row->deleteLater();
}

void FaultInjectorDialog::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    QPalette pal = palette();
    pal.setColor(QPalette::Window, colors.surfaceMain);
    setPalette(pal);

    QPixmap pixmap(Constants::FAULT_INJECTOR_CONFIRM_ICON_PATH);
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
        #faultInjectorDialog {
            background-color: %1;
            color: %2;
        }
        #faultInjectorDialog QPushButton {
            background-color: %3;
            color: %2;
            border: none;
            border-radius: %4px;
            padding: %5px %6px;
            font-weight: %7;
            font-size: %8px;
            text-align: center;
        }
        #faultInjectorDialog QPushButton:hover {
            background-color: %9;
        }
        #faultInjectorDialog QPushButton:pressed {
            background-color: %9;
        }
        #faultInjectorDialog QScrollArea,
        #faultInjectorDialog QScrollArea > QWidget {
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
}

}  // namespace FaultInjector