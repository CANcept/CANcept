#include "styled_filter_bar.hpp"

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace Core {
StyledFilterBar::StyledFilterBar(QWidget* parent) : QWidget(parent)
{
    setupUi();
    setupStyles();
}

// --- Getter ---

auto StyledFilterBar::searchText() const -> QString
{
    return m_searchBar->text();
}

auto StyledFilterBar::currentFilter() const -> QString
{
    return m_filterBox->currentText();
}

// --- Setter ---

void StyledFilterBar::setPlaceholderText(const QString& text) const
{
    m_searchBar->setPlaceholderText(text);
}

void StyledFilterBar::setSearchText(const QString& text) const
{
    m_searchBar->setText(text);
}

void StyledFilterBar::setFilterOptions(const QStringList& options) const
{
    m_filterBox->clear();
    m_filterBox->addItems(options);
}

void StyledFilterBar::setCurrentFilter(const QString& text) const
{
    m_filterBox->setCurrentText(text);
}

void StyledFilterBar::setCurrentFilterIndex(int index) const
{
    m_filterBox->setCurrentIndex(index);
}

// -----------------------------------------------------------------------------
// UI
// -----------------------------------------------------------------------------

void StyledFilterBar::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Search field ---------------------------------------------------------
    m_searchBar = new QLineEdit(this);
    m_searchBar->setObjectName("SearchField");

    // Icon
    auto* searchAction = new QAction(m_searchBar);
    searchAction->setIcon(QIcon(Constants::SEARCH_ICON));
    m_searchBar->addAction(searchAction, QLineEdit::LeadingPosition);

    // --- Filter combo ---------------------------------------------------------
    m_filterBox = new QComboBox(this);
    m_filterBox->setObjectName("FilterCombo");
    m_filterBox->setView(new QListView(m_filterBox));
    // --- Layout ---------------------------------------------------------------
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    layout->addWidget(m_searchBar, 3);
    layout->addWidget(m_filterBox, 1);

    // --- Signals --------------------------------------------------------------
    connect(m_searchBar, &QLineEdit::textChanged, this, &StyledFilterBar::searchTextChanged);

    connect(m_filterBox, &QComboBox::currentIndexChanged, this,
            &StyledFilterBar::filterIndexChanged);
}

// -----------------------------------------------------------------------------
// Styles
// -----------------------------------------------------------------------------

void StyledFilterBar::setupStyles() const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // --- Search ---------------------------------------------------------------
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
                                    .arg(colors.surfacePrimary.name())  // %1
                                    .arg(spacing.radiusMd)              // %2
                                    .arg(32)                            // %3
                                    .arg(spacing.spacingLg)             // %4
                                    .arg(spacing.spacingMd)             // %5
                                    .arg(colors.textPrimary.name())     // %6
                                    .arg(spacing.fontSizeSm)            // %7
                                    .arg(colors.textSecondary.name());  // %8

    m_searchBar->setStyleSheet(searchStyle);

    // --- ComboBox -------------------------------------------------------------
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
                                   .arg(colors.surfacePrimary.name())  // %1
                                   .arg(colors.textSecondary.name())   // %2
                                   .arg(spacing.radiusMd)              // %3
                                   .arg(32)                            // %4
                                   .arg(spacing.spacingLg)             // %5
                                   .arg(spacing.spacingXl * 2)         // %6
                                   .arg(spacing.fontSizeSm)            // %7
                                   .arg(Constants::ARROW_DOWN_ICON)    // %8
                                   .arg(spacing.spacingSm)             // %9
                                   .arg(spacing.radiusSm)              // %10
                                   .arg(colors.surfaceMain.name());    // %11

    m_filterBox->setStyleSheet(comboStyle);
}
}  // namespace Core