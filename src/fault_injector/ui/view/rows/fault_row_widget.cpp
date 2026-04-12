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

#include "fault_row_widget.hpp"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "widget_utils.hpp"

namespace FaultInjector {

FaultRowWidget::FaultRowWidget(Providers providers, const bool showRemove, QWidget* parent)
    : QWidget(parent), m_providers(std::move(providers))
{
    const auto& spacing = THEME.spacing();

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    const int rowHeight = spacing.HeightSm;

    m_typeCombo = new Core::StyledComboBox(this);
    for (const auto& p : m_providers)
    {
        m_typeCombo->addItem(p->typeName());
    }
    m_typeCombo->setFixedWidth(spacing.spacingXl * 4);
    m_typeCombo->setFixedHeight(rowHeight);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &FaultRowWidget::onTypeChanged);
    layout->addWidget(m_typeCombo);

    m_optionsLayout = new QHBoxLayout();
    m_optionsLayout->setContentsMargins(0, 0, 0, 0);
    m_optionsLayout->setSpacing(spacing.spacingSm);
    layout->addLayout(m_optionsLayout, 1);

    m_removeButton = new QPushButton("X", this);
    m_removeButton->setFixedHeight(rowHeight);
    m_removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_removeButton->setVisible(showRemove);
    connect(m_removeButton, &QPushButton::clicked, this, &FaultRowWidget::removeRequested);
    layout->addWidget(m_removeButton);

    if (!m_providers.empty())
    {
        onTypeChanged(0);
    }

    applyStyle();
}

auto FaultRowWidget::currentEntry() const -> SectionEntry
{
    SectionEntry entry;
    entry.typeIndex = m_typeCombo->currentIndex();

    if (m_currentOptions)
    {
        for (auto* spin : collectWidgets<QSpinBox>(m_currentOptions))
        {
            if (!spin->objectName().isEmpty())
            {
                entry.params[spin->objectName()] = spin->value();
            }
        }
        for (auto* spin : collectWidgets<QDoubleSpinBox>(m_currentOptions))
        {
            if (!spin->objectName().isEmpty())
            {
                entry.params[spin->objectName()] = spin->value();
            }
        }
        for (auto* edit : collectWidgets<QLineEdit>(m_currentOptions))
        {
            if (!edit->objectName().isEmpty())
            {
                entry.params[edit->objectName()] = edit->text();
            }
        }
    }

    return entry;
}

void FaultRowWidget::setFromEntry(const SectionEntry& entry)
{
    const QSignalBlocker blocker(this);

    if (entry.typeIndex >= 0 && entry.typeIndex < m_typeCombo->count())
    {
        m_typeCombo->setCurrentIndex(entry.typeIndex);
    }

    if (!m_currentOptions)
    {
        return;
    }

    for (auto* spin : collectWidgets<QSpinBox>(m_currentOptions))
    {
        if (entry.params.contains(spin->objectName()))
        {
            spin->setValue(entry.params[spin->objectName()].toInt());
        }
    }
    for (auto* spin : collectWidgets<QDoubleSpinBox>(m_currentOptions))
    {
        if (entry.params.contains(spin->objectName()))
        {
            spin->setValue(entry.params[spin->objectName()].toDouble());
        }
    }
    for (auto* edit : collectWidgets<QLineEdit>(m_currentOptions))
    {
        if (entry.params.contains(edit->objectName()))
        {
            edit->setText(entry.params[edit->objectName()].toString());
        }
    }
}

void FaultRowWidget::onTypeChanged(const int index)
{
    if (m_currentOptions)
    {
        m_optionsLayout->removeWidget(m_currentOptions);
        delete m_currentOptions;
        m_currentOptions = nullptr;
    }

    if (index < 0 || static_cast<std::size_t>(index) >= m_providers.size())
    {
        return;
    }

    m_currentOptions = m_providers[static_cast<std::size_t>(index)]->createOptionsWidget(this);
    if (m_currentOptions)
    {
        m_typeCombo->setFixedWidth(THEME.spacing().spacingXl * 4);
        m_optionsLayout->addWidget(m_currentOptions);
        connectOptionsSignals();
        applyStyle();
    } else
    {
        m_typeCombo->setMaximumWidth(QWIDGETSIZE_MAX);
        m_typeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    emit changed();
}

void FaultRowWidget::connectOptionsSignals() const
{
    if (!m_currentOptions)
    {
        return;
    }
    for (const auto* spin : collectWidgets<QSpinBox>(m_currentOptions))
    {
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FaultRowWidget::changed);
    }
    for (const auto* spin : collectWidgets<QDoubleSpinBox>(m_currentOptions))
    {
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaultRowWidget::changed);
    }
    for (const auto* edit : collectWidgets<QLineEdit>(m_currentOptions))
    {
        connect(edit, &QLineEdit::textChanged, this, &FaultRowWidget::changed);
    }
}

void FaultRowWidget::applyStyle() const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    const QString comboStyle = QString(
                                   "QComboBox {"
                                   "  background-color: %1;"
                                   "  color: %2;"
                                   "  border-radius: %3px;"
                                   "  padding: %4px %5px;"
                                   "  font-size: %6px;"
                                   "}"
                                   "QComboBox:hover {"
                                   "  background-color: %7;"
                                   "}"
                                   "QComboBox::drop-down {"
                                   "  border: none;"
                                   "  width: %8px;"
                                   "  padding-right: %5px;"
                                   "}"
                                   "QComboBox::down-arrow {"
                                   "  image: url(%10);"
                                   "}"
                                   "QComboBox QAbstractItemView {"
                                   "  background-color: %7;"
                                   "  color: %2;"
                                   "  border: none;"
                                   "  border-radius: %9px;"
                                   "  outline: none;"
                                   "  padding: %4px;"
                                   "}")
                                   .arg(colors.surfaceSecondary.name())
                                   .arg(colors.textPrimary.name())
                                   .arg(spacing.radiusSm)
                                   .arg(spacing.spacingXs)
                                   .arg(spacing.spacingSm)
                                   .arg(spacing.fontSizeSm)
                                   .arg(colors.surfacePrimary.name())
                                   .arg(spacing.spacingLg)
                                   .arg(spacing.radiusSm)
                                   .arg(Core::Constants::ARROW_DOWN_ICON);

    m_typeCombo->setStyleSheet(comboStyle);

    if (m_currentOptions)
    {
        for (auto* combo : m_currentOptions->findChildren<QComboBox*>())
        {
            combo->setStyleSheet(comboStyle);
            combo->setFixedHeight(spacing.HeightSm);
        }

        const QString inputStyle = QString(
                                       "QSpinBox, QDoubleSpinBox, QLineEdit {"
                                       "  background-color: %1;"
                                       "  color: %2;"
                                       "  border-radius: %3px;"
                                       "  padding: %4px %5px;"
                                       "  font-size: %6px;"
                                       "  border: none;"
                                       "}"
                                       "QSpinBox:hover, QDoubleSpinBox:hover, QLineEdit:hover {"
                                       "  background-color: %7;"
                                       "}"
                                       "QSpinBox::up-button, QSpinBox::down-button,"
                                       "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
                                       "  width: 0px; border: none;"
                                       "}")
                                       .arg(colors.surfaceSecondary.name())
                                       .arg(colors.textPrimary.name())
                                       .arg(spacing.radiusSm)
                                       .arg(spacing.spacingXs)
                                       .arg(spacing.spacingSm)
                                       .arg(spacing.fontSizeSm)
                                       .arg(colors.surfacePrimary.name());

        m_currentOptions->setStyleSheet(inputStyle);
        m_currentOptions->setFixedHeight(spacing.HeightSm);

        for (auto* label : m_currentOptions->findChildren<QLabel*>())
        {
            label->setStyleSheet(QString("color: %1; font-size: %2px; background: transparent;")
                                     .arg(colors.textSecondary.name())
                                     .arg(spacing.fontSizeSm));
        }
    }

    m_removeButton->setStyleSheet(QString("QPushButton {"
                                          "  background-color: transparent;"
                                          "  border: none;"
                                          "  font-size: %1px;"
                                          "  font-weight: %2;"
                                          "  color: %3;"
                                          "}"
                                          "QPushButton:hover {"
                                          "  background-color: %4;"
                                          "  border-radius: %5px;"
                                          "}")
                                      .arg(spacing.fontSizeSm)
                                      .arg(spacing.fontWeightNormal)
                                      .arg(colors.textPrimary.name())
                                      .arg(colors.surfaceHover.name())
                                      .arg(spacing.radiusSm));
}

auto FaultRowWidget::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace FaultInjector