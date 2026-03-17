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

//
// Created by Adrian Rupp on 20.01.26.
//

#include "searchable_filter_widgets.hpp"

#include <QHeaderView>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"

namespace Core {
// --- SearchableFilterTable ---
SearchableFilterTable::SearchableFilterTable(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

auto SearchableFilterTable::tableView() const -> QTableView*
{
    return m_tableView;
}

auto SearchableFilterTable::filterBar() const -> Core::StyledFilterBar*
{
    return m_filterBar;
}

void SearchableFilterTable::setSearchPlaceholder(const QString& text) const
{
    m_filterBar->setPlaceholderText(text);
}

void SearchableFilterTable::setFilterOptions(const QStringList& options) const
{
    m_filterBar->setFilterOptions(options);
}

void SearchableFilterTable::setSearchText(const QString& text) const
{
    m_filterBar->setSearchText(text);
}

void SearchableFilterTable::applyStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // === FRAME handles radius + border + background ===
    const QString frameQss = QString(R"(
        QFrame#tableFrame {
            background-color: %1;
            border: %2px solid %3;
            border-radius: %4px;
        }
    )")
                                 .arg(colors.surfaceMain.name(QColor::HexArgb))
                                 .arg(spacing.borderThin)
                                 .arg(colors.borderSubtle.name(QColor::HexArgb))
                                 .arg(spacing.radiusSm);

    m_tableFrame->setStyleSheet(frameQss);

    // === TABLE completely transparent ===
    const QString tableQss = QString(R"(

        QTableView {
            border: none;
            background: transparent;
            color: %1;
            selection-background-color: %2;
            selection-color: %1;
        }

        QTableView::viewport {
            background: transparent;
        }

        QTableView::item {
            background: transparent;
            padding: %3px;
        }

        QTableView::item:selected {
            background-color: %2;
            color: %1;
        }

        QHeaderView {
            background: transparent;
        }

        QHeaderView::section {
            background: transparent;
            color: %1;
            border: none;
            border-bottom: %4px solid %5;
            padding: %3px;
            font-weight: bold;
        }

    )")
                                 .arg(colors.textPrimary.name())
                                 .arg(colors.surfaceSelected.name())
                                 .arg(spacing.spacingXs)
                                 .arg(spacing.borderThick)
                                 .arg(colors.borderSubtle.name(QColor::HexArgb));

    m_tableView->setStyleSheet(tableQss);

    update();
}
void SearchableFilterTable::configureTableBasics()
{
    // Scrollbars
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Remove default frame & grid
    m_tableView->setFrameStyle(QFrame::NoFrame);
    m_tableView->setShowGrid(false);
    m_tableView->setAlternatingRowColors(false);

    // Hide vertical header
    if (m_tableView->verticalHeader()) m_tableView->verticalHeader()->hide();

    // VERY IMPORTANT:
    // Prevent header from painting opaque background
    m_tableView->horizontalHeader()->setAutoFillBackground(false);
    m_tableView->horizontalHeader()->setAttribute(Qt::WA_NoSystemBackground);
    m_tableView->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);

    // Prevent table from painting over frame
    m_tableView->setAttribute(Qt::WA_NoSystemBackground);
    m_tableView->setAttribute(Qt::WA_TranslucentBackground);
    m_tableView->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    m_tableView->viewport()->setAttribute(Qt::WA_TranslucentBackground);
}

void SearchableFilterTable::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Main Layout ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(spacing.spacingLg);

    // --- Filter Bar ---
    m_filterBar = new Core::StyledFilterBar(this);

    // --- Outer Frame (handles border + radius + background) ---
    m_tableFrame = new QFrame(this);
    m_tableFrame->setObjectName("tableFrame");

    // --- Table View ---
    m_tableView = new QTableView(m_tableFrame);
    m_tableView->setFrameStyle(QFrame::NoFrame);

    auto* frameLayout = new QVBoxLayout(m_tableFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(m_tableView);

    mainLayout->addWidget(m_filterBar);
    mainLayout->addWidget(m_tableFrame);

    configureTableBasics();

    // --- Forward filter signals ---
    connect(m_filterBar, &StyledFilterBar::searchTextChanged, this,
            &SearchableFilterTable::filterTextChanged);

    connect(m_filterBar, &StyledFilterBar::filterIndexChanged, this,
            &SearchableFilterTable::filterIndexChanged);

    applyStyle();
}
auto SearchableFilterTable::event(QEvent* event) -> bool
{
    if (event->type() == StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

// --- SearchableFilterTree ---
SearchableFilterTree::SearchableFilterTree(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
auto SearchableFilterTree::treeView() const -> QTreeView*
{
    return m_treeView;
}

auto SearchableFilterTree::filterBar() const -> Core::StyledFilterBar*
{
    return m_filterBar;
}

void SearchableFilterTree::setSearchPlaceholder(const QString& text) const
{
    m_filterBar->setPlaceholderText(text);
}

void SearchableFilterTree::setFilterOptions(const QStringList& options) const
{
    m_filterBar->setFilterOptions(options);
}

void SearchableFilterTree::setSearchText(const QString& text) const
{
    m_filterBar->setSearchText(text);
}

void SearchableFilterTree::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Main Layout setup ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- FilterBar ---
    m_filterBar = new StyledFilterBar(this);

    // --- Tree View ---
    m_treeView = new QTreeView(this);
    m_treeView->setFrameStyle(QFrame::NoFrame);
    mainLayout->addWidget(m_filterBar);
    mainLayout->addWidget(m_treeView);

    // Signal forwarding
    connect(m_filterBar, &Core::StyledFilterBar::searchTextChanged, this,
            &SearchableFilterTree::filterTextChanged);

    connect(m_filterBar, &Core::StyledFilterBar::filterIndexChanged, this,
            &SearchableFilterTree::filterIndexChanged);
}

}  // namespace Core