//
// Created by Adrian Rupp on 20.01.26.
//

#include "searchable_filter_widgets.hpp"

#include <QHeaderView>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
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
void SearchableFilterTable::configureHeaderStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

    auto* header = m_tableView->horizontalHeader();
    if (!header) return;

    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(false);

    header->setStyleSheet(QString(
        "QHeaderView::section {"
        "   background-color: %1;"
        "   border: none;"
        "   border-bottom: %2px solid %3;"
        "   padding: %4px;"
        "   font-weight: bold;"
        "   color: %5;"
        "}")
        .arg(colors.surfaceMain.name())
        .arg(spacing.borderThick)
        .arg(colors.borderSubtle.name(QColor::HexArgb))
        .arg(spacing.spacingXs)
        .arg(colors.textPrimary.name())
    );
}
void SearchableFilterTable::applyTableStyle()
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

    m_tableView->setStyleSheet(QString(
        "QTableView {"
        "   border: none;"
        "   border-radius: %1px;"
        "   background-color: %2;"
        "}")
        .arg(spacing.radiusSm)
        .arg(colors.surfaceMain.name())
    );
}
void SearchableFilterTable::configureTableBasics()
{
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_tableView->setShowGrid(false);
    m_tableView->setAlternatingRowColors(false);

    if (m_tableView->verticalHeader())
        m_tableView->verticalHeader()->hide();
}


void SearchableFilterTable::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // --- Main Layout setup ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(spacing.spacingLg);

    // --- Top bar setup ---
    m_filterBar = new Core::StyledFilterBar(this);

    // --- Table View ---
    auto* borderFrame = new QFrame(this);
    borderFrame->setObjectName("tableFrame");

    borderFrame->setStyleSheet(QString(
        "QFrame#tableFrame {"
        "background: transparent;"
        "border: %1px solid %2;"
        "border-radius: %3px;"
        "}")
        .arg(spacing.borderThin)
        .arg(colors.borderSubtle.name(QColor::HexArgb))
        .arg(spacing.radiusSm)
    );

    m_tableView = new QTableView(borderFrame);
    m_tableView->setFrameStyle(QFrame::NoFrame);

    auto* borderLayout = new QVBoxLayout(borderFrame);
    borderLayout->addWidget(m_tableView);

    mainLayout->addWidget(m_filterBar);
    mainLayout->addWidget(borderFrame);
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