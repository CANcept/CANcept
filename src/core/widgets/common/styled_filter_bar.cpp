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

#include "styled_filter_bar.hpp"

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "styled_combo_box.hpp"

namespace Core {

StyledFilterBar::StyledFilterBar(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyle();
}

// --- Getter ---

auto StyledFilterBar::searchText() const -> QString
{
    return m_searchBar->text();
}

auto StyledFilterBar::currentFilterText() const -> QString
{
    return m_filterBox->currentText();
}

auto StyledFilterBar::currentFilterData() const -> QVariant
{
    if (!m_filterBox) return {};
    return m_filterBox->currentData();
}

// --- Setter ---

void StyledFilterBar::setPlaceholderText(const QString& text)
{
    m_searchBar->setPlaceholderText(text);
}

void StyledFilterBar::setSearchText(const QString& text)
{
    m_searchBar->setText(text);
}

void StyledFilterBar::clearFilterOptions()
{
    if (m_filterBox) m_filterBox->clear();
}

void StyledFilterBar::setFilterOptions(const QStringList& options)
{
    m_filterBox->clear();
    m_filterBox->addItems(options);
}

void StyledFilterBar::addFilterOption(const QString& text, const QVariant& userData)
{
    if (m_filterBox) m_filterBox->addItem(text, userData);
}

void StyledFilterBar::setCurrentFilterText(const QString& text)
{
    if (!m_filterBox) return;
    int idx = m_filterBox->findText(text);
    if (idx >= 0)
        m_filterBox->setCurrentIndex(idx);
    else
        m_filterBox->setCurrentIndex(0);  // Fallback
}
void StyledFilterBar::setCurrentFilterIndex(int index)
{
    m_filterBox->setCurrentIndex(index);
}

// -----------------------------------------------------------------------------
// UI
// -----------------------------------------------------------------------------

void StyledFilterBar::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Search field -------------------------------------------------
    m_searchBar = new QLineEdit(this);
    m_searchBar->setObjectName("SearchField");
    m_searchBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* searchAction = new QAction(m_searchBar);
    searchAction->setIcon(QIcon(Constants::SEARCH_ICON));
    m_searchBar->addAction(searchAction, QLineEdit::LeadingPosition);

    // --- Filter combo -------------------------------------------------
    m_filterBox = new QComboBox(this);
    m_filterBox->setObjectName("FilterCombo");
    m_filterBox->setView(new QListView(m_filterBox));
    m_filterBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // --- Layout -------------------------------------------------------
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    layout->addWidget(m_searchBar, 3);
    layout->addWidget(m_filterBox, 1);

    // --- Signals ------------------------------------------------------
    connect(m_searchBar, &QLineEdit::textChanged, this, &StyledFilterBar::searchTextChanged);

    connect(m_filterBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &StyledFilterBar::filterIndexChanged);
}

void StyledFilterBar::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // =========================
    // Search Field Styling
    // =========================
    const QString searchStyle = QString(R"(
QLineEdit#SearchField {
    background-color: %1;
    border: none;
    border-radius: %2px;

    min-height: %3px;
    padding-left: %4px;
    padding-right: %5px;

    color: %6;
    font-size: %7px;
}

QLineEdit#SearchField::placeholder {
    color: %8;
}
)")
                                    .arg(colors.surfaceSecondary.name())
                                    .arg(spacing.radiusMd)
                                    .arg(spacing.HeightSm)
                                    .arg(spacing.spacingLg)
                                    .arg(spacing.spacingMd)
                                    .arg(colors.textPrimary.name())
                                    .arg(spacing.fontSizeSm)
                                    .arg(colors.textSecondary.name());

    m_searchBar->setStyleSheet(searchStyle);

    // =========================
    // ComboBox Styling
    // =========================
    const QString comboStyle = QString(R"(
QComboBox#FilterCombo {
    background-color: %1;
    color: %2;

    border: none;
    border-radius: %3px;

    min-height: %4px;
    padding-left: %5px;
    padding-right: %6px;

    font-size: %7px;
    combobox-popup: 0;
}

QComboBox#FilterCombo:on {
    border-bottom-left-radius: 0px;
    border-bottom-right-radius: 0px;
}

QComboBox#FilterCombo::drop-down {
    border: none;
    width: %6px;
}

QComboBox#FilterCombo::down-arrow {
    image: url(%8);
}

QComboBox#FilterCombo QAbstractItemView {
    background-color: %1;
    color: %2;

    border: none;
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
    border-bottom-left-radius: %3px;
    border-bottom-right-radius: %3px;
    padding: %9px;

    outline: none;
    selection-background-color: %10;
}

QComboBox#FilterCombo QAbstractItemView::item {
    padding: %9px;
    border-radius: %10px;
}

QComboBox#FilterCombo QAbstractItemView::item:hover,
QComboBox#FilterCombo QAbstractItemView::item:selected {
    background-color: %11;
    color: %2;
}
)")
                                   .arg(colors.surfaceSecondary.name())
                                   .arg(colors.textSecondary.name())
                                   .arg(spacing.radiusMd)
                                   .arg(spacing.HeightSm)
                                   .arg(spacing.spacingLg)
                                   .arg(spacing.spacingXl * 2)
                                   .arg(spacing.fontSizeSm)
                                   .arg(Constants::ARROW_DOWN_ICON)
                                   .arg(spacing.spacingSm)
                                   .arg(spacing.radiusSm)
                                   .arg(colors.surfaceMain.name());

    m_filterBox->setStyleSheet(comboStyle);

    update();
}
auto StyledFilterBar::event(QEvent* event) -> bool
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

}  // namespace Core