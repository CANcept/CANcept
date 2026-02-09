#include "styled_filter_bar.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QListView>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"

namespace Core {

StyledFilterBar::StyledFilterBar(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    setupStyles();
}

// --- Getter ---

QString StyledFilterBar::searchText() const
{
    return m_searchBar->text();
}

QString StyledFilterBar::currentFilter() const
{
    return m_filterBox->currentText();
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

void StyledFilterBar::setFilterOptions(const QStringList& options)
{
    m_filterBox->clear();
    m_filterBox->addItems(options);
}

void StyledFilterBar::setCurrentFilter(const QString& text)
{
    m_filterBox->setCurrentText(text);
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

    // --- Search field ---------------------------------------------------------
    m_searchBar = new QLineEdit(this);
    m_searchBar->setObjectName("SearchField");
    m_searchBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Search icon
    auto* searchAction = new QAction(m_searchBar);
    searchAction->setIcon(QIcon(Constants::SEARCH_ICON));
    m_searchBar->addAction(searchAction, QLineEdit::LeadingPosition);

    // --- Filter combo ---------------------------------------------------------
    m_filterBox = new QComboBox(this);
    m_filterBox->setObjectName("FilterCombo");
    m_filterBox->setView(new QListView(m_filterBox));
    m_filterBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // --- Layout ---------------------------------------------------------------
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(spacing.spacingSm);

    layout->addWidget(m_searchBar, 3);
    layout->addWidget(m_filterBox, 1);

    // --- Signals --------------------------------------------------------------
    connect(m_searchBar, &QLineEdit::textChanged,
            this, &StyledFilterBar::searchTextChanged);

    connect(m_filterBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StyledFilterBar::filterIndexChanged);
}

// -----------------------------------------------------------------------------
// Styles
// -----------------------------------------------------------------------------

void StyledFilterBar::setupStyles()
{
    const auto& spacing = THEME.spacing();
    const auto& colors  = THEME.colors();

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
        .arg(colors.surfacePrimary.name())
        .arg(spacing.radiusMd)
        .arg(spacing.HeightSm)
        .arg(spacing.spacingLg)
        .arg(spacing.spacingMd)
        .arg(colors.textPrimary.name())
        .arg(spacing.fontSizeSm)
        .arg(colors.textSecondary.name());

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
        .arg(colors.surfacePrimary.name())
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
}

} // namespace Core