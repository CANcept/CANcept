#include "signals_page.hpp"
#include <QHeaderView>

#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/signal_table_delegate.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

// ============================================================================
// Constructor
// ============================================================================

SignalsPage::SignalsPage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

// ============================================================================
// Public API
// ============================================================================

void SignalsPage::setModel(QAbstractItemModel* model) const
{
    if (!m_tableWidget) return;

    auto* table = m_tableWidget->tableView();
    if (!table) return;

    table->setModel(model);
    configureColumns(table, model);
}

void SignalsPage::setAvailableUnits(const QStringList& units) const
{
    if (!m_tableWidget) return;

    auto* bar = m_tableWidget->filterBar();
    if (!bar) return;

    const bool wasBlocked = bar->blockSignals(true);

    QString currentSelection = bar->currentFilterText();

    bar->clearFilterOptions();
    bar->addFilterOption(Constants::SignalsPage::FilterAllText, QString());

    for (const auto& unit : units)
        bar->addFilterOption(unit, unit);

    bar->setCurrentFilterText(currentSelection);
    bar->blockSignals(wasBlocked);
}

// ============================================================================
// UI Setup
// ============================================================================

void SignalsPage::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd,
                                   spacing.spacingMd,
                                   spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- Header Card ---------------------------------------------------------

    auto* card = new Core::CardWidget(Constants::SignalsPage::PageHeaderTitle,
                                      Constants::SignalsPage::PageHeaderSubtitle,
                                      "", this);

    mainLayout->addWidget(card);

    // --- Searchable Table ----------------------------------------------------

    m_tableWidget = new Core::SearchableFilterTable(this);
    m_tableWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);

    m_tableWidget->configureTableBasics();
    m_tableWidget->applyTableStyle();
    m_tableWidget->configureHeaderStyle();

    card->layout()->addWidget(m_tableWidget);

    auto* table = m_tableWidget->tableView();

    table->setItemDelegate(new SignalTableDelegate(table));
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);


    // --- Signal Forwarding ---------------------------------------------------

    connect(m_tableWidget, &Core::SearchableFilterTable::filterTextChanged,
            this, &SignalsPage::filterTextChanged);

    connect(m_tableWidget, &Core::SearchableFilterTable::filterIndexChanged,
            this, &SignalsPage::onFilterIndexChanged);
}

// ============================================================================
// Table Configuration (UI only)
// ============================================================================

// void SignalsPage::configureTableBasics(QTableView* table)
// {
//     table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//     table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//
//     table->setShowGrid(false);
//     table->setAlternatingRowColors(false);
//
//     table->setSelectionMode(QAbstractItemView::NoSelection);
//     table->setSelectionBehavior(QAbstractItemView::SelectRows);
//
//     if (table->verticalHeader())
//         table->verticalHeader()->hide();
// }

// void SignalsPage::applyTableStyle(QTableView* table)
// {
//     const auto& spacing = THEME.spacing();
//     const auto& colors  = THEME.colors();
//
//     table->setStyleSheet(QString(
//         "QTableView {"
//         "   border: none;"
//         "   border-radius: %1px;"
//         "   background-color: %2;"
//         "}")
//         .arg(spacing.radiusSm)
//         .arg(colors.surfaceMain.name())
//     );
// }
//
// void SignalsPage::configureHeaderStyle(const QTableView* table)
// {
//     const auto& spacing = THEME.spacing();
//     const auto& colors  = THEME.colors();
//
//     auto* header = table->horizontalHeader();
//     if (!header) return;
//
//     header->setSectionResizeMode(QHeaderView::Interactive);
//     header->setStretchLastSection(false);
//
//     header->setStyleSheet(QString(
//         "QHeaderView::section {"
//         "   background-color: %1;"
//         "   border: none;"
//         "   border-bottom: %2px solid %3;"
//         "   padding: %4px;"
//         "   font-weight: bold;"
//         "   color: %5;"
//         "}")
//         .arg(colors.surfaceMain.name())
//         .arg(spacing.borderThick)
//         .arg(colors.borderSubtle.name(QColor::HexArgb))
//         .arg(spacing.spacingXs)
//         .arg(colors.textPrimary.name())
//     );
// }

// ============================================================================
// Model-dependent Column Configuration
// ============================================================================

void SignalsPage::configureColumns(QTableView* table,
                                   const QAbstractItemModel* model)
{
    if (!model || model->columnCount() == 0)
        return;

    const auto& spacing = THEME.spacing();
    auto* header = table->horizontalHeader();

    // Hide columns safely
    auto hideIfExists = [&](int column) {
        if (model->columnCount() > column)
            table->setColumnHidden(column, true);
    };

    hideIfExists(Constants::Columns::SigMax);
    hideIfExists(Constants::Columns::SigValueType);
    hideIfExists(Constants::Columns::SigReceivers);

    // Column widths
    table->setColumnWidth(Constants::Columns::SigStartBit, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigUnit,     spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigLength,   spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigMin,      spacing.WidthXs);
    table->setColumnWidth(Constants::Columns::SigFactor,   spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigOffset,   spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigByteOrder, spacing.WidthXs);
    table->setColumnWidth(Constants::Columns::SigValueType, spacing.WidthXs / 2);

    // Stretch important columns
    header->setSectionResizeMode(Constants::Columns::SigName, QHeaderView::Stretch);
    header->setSectionResizeMode(Constants::Columns::SigMessage, QHeaderView::Stretch);
}

// ============================================================================
// Slots
// ============================================================================

void SignalsPage::onFilterIndexChanged(int)
{
    const QString unit =
        m_tableWidget->filterBar()->currentFilterData().toString();

    emit filterUnitChanged(unit);
}

} // namespace DbcFile