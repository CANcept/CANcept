//
// Created by Adrian Rupp on 20.01.26.
//

#include "searchable_filter_widgets.hpp"

#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"
#include "dbc_file/constants.hpp"

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

void SearchableFilterTable::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Main Layout setup ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(spacing.spacingLg);

    // --- Top bar setup ---
    m_filterBar = new Core::StyledFilterBar(this);

    // --- Table View ---
    m_tableView = new QTableView(this);
    m_tableView->setFrameStyle(QFrame::NoFrame);

    mainLayout->addWidget(m_filterBar);
    mainLayout->addWidget(m_tableView);

    // Signal forwarding
    connect(m_filterBar, &Core::StyledFilterBar::searchTextChanged, this,
            &SearchableFilterTable::filterTextChanged);

    connect(m_filterBar, &Core::StyledFilterBar::filterIndexChanged, this,
            &SearchableFilterTable::filterIndexChanged);
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
    m_filterBar = new Core::StyledFilterBar(this);

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