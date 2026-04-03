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

#include "dbc_signal_row.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "card_widget.hpp"
#include "common/styled_checkbox.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace Core {

DbcSignalRowWidget::DbcSignalRowWidget(Math::VariableRegistry& registry, const QString& name,
                                       const QString& unit, const double min, const double max,
                                       QWidget* parent)
    : QWidget(parent),
      m_registry(&registry),
      m_cardContainer(nullptr),
      m_selectionCheckbox(nullptr),
      m_nameLabel(nullptr),
      m_rangeLabel(nullptr),
      m_valueEditor(nullptr),
      m_unitLabel(nullptr)
{
    Config defaultConfig;
    defaultConfig.mode = Mode::Full;
    defaultConfig.showRange = true;
    defaultConfig.showSelectionCheckbox = false;
    setupUi(name, unit, min, max, defaultConfig);
}

DbcSignalRowWidget::DbcSignalRowWidget(const QString& name, const QString& unit, double min,
                                       double max, const Config& config, QWidget* parent)
    : QWidget(parent),
      m_registry(nullptr),
      m_cardContainer(nullptr),
      m_selectionCheckbox(nullptr),
      m_nameLabel(nullptr),
      m_rangeLabel(nullptr),
      m_valueEditor(nullptr),
      m_unitLabel(nullptr)
{
    setupUi(name, unit, min, max, config);
}

void DbcSignalRowWidget::setupUi(const QString& name, const QString& unit, double min, double max,
                                 const Config& config)
{
    if (config.mode == Mode::Full)
    {
        setupFullMode(name, unit, min, max, config);
    } else
    {
        setupSelectionMode(name, unit, config);
    }
}

void DbcSignalRowWidget::setupFullMode(const QString& name, const QString& unit, const double min,
                                       const double max, const Config& config)
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_cardContainer = new CardWidget(QString(), QString(), QString(), this);
    auto* cardLayout = m_cardContainer->contentLayout();

    if (!cardLayout)
    {
        return;
    }

    cardLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    cardLayout->setSpacing(spacing.spacingSm);

    auto* firstRow = new QHBoxLayout();
    firstRow->setSpacing(spacing.spacingMd);

    m_selectionCheckbox = new StyledCheckBox(m_cardContainer);
    m_selectionCheckbox->setChecked(false);
    firstRow->addWidget(m_selectionCheckbox);

    m_nameLabel = new QLabel(name, m_cardContainer);
    firstRow->addWidget(m_nameLabel);
    firstRow->addStretch();

    if (config.showRange)
    {
        const QString rangeText =
            QString("%1-%2 %3").arg(min, 0, 'f', 0).arg(max, 0, 'f', 0).arg(unit);
        m_rangeLabel = new QLabel(rangeText, m_cardContainer);
        firstRow->addWidget(m_rangeLabel);
    }
    cardLayout->addLayout(firstRow);
    cardLayout->addSpacing(spacing.spacingXs);

    m_valueEditor = new Math::MathInputView(*m_registry, m_cardContainer);
    cardLayout->addWidget(m_valueEditor);

    mainLayout->addWidget(m_cardContainer);

    applyStyle();
}

void DbcSignalRowWidget::setupSelectionMode(const QString& name, const QString& unit,
                                            const Config& config)
{
    const auto& spacing = THEME.spacing();

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, spacing.spacingXs / 2, 0, spacing.spacingXs / 2);
    layout->setSpacing(spacing.spacingSm);

    // Optional styled selection checkbox
    if (config.showSelectionCheckbox)
    {
        m_selectionCheckbox = new StyledCheckBox(this);
        layout->addWidget(m_selectionCheckbox);
    }

    // Signal name
    m_nameLabel = new QLabel(name, this);
    layout->addWidget(m_nameLabel);

    layout->addStretch();

    // Unit label (if provided)
    if (!unit.isEmpty())
    {
        m_unitLabel = new QLabel(unit, this);
        layout->addWidget(m_unitLabel);
    }

    applyStyle();
}

void DbcSignalRowWidget::applyStyle() const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    if (m_nameLabel)
    {
        m_nameLabel->setStyleSheet(
            QString("QLabel { font-size: %1px; font-weight: %2; color: %3; }")
                .arg(spacing.fontSizeSm)
                .arg(spacing.fontWeightNormal)
                .arg(colors.textPrimary.name()));
    }

    if (m_rangeLabel)
    {
        m_rangeLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2px; }")
                                        .arg(colors.textDisabled.name())
                                        .arg(spacing.fontSizeXs));
    }

    if (m_unitLabel)
    {
        m_unitLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2px; }")
                                       .arg(colors.textSecondary.name())
                                       .arg(spacing.fontSizeSm));
    }
}

bool DbcSignalRowWidget::event(QEvent* event)
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Core
