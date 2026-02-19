#include "signals_page.hpp"

#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/signal_table_delegate.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/styles.hpp"

namespace DbcFile {

// ============================================================================
// Constructor
// ============================================================================
SignalsPage::SignalsPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyle();
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

    connect(model, &QAbstractItemModel::modelReset, this, &SignalsPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsInserted, this, &SignalsPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &SignalsPage::updateEmptyState);
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

    for (const auto& unit : units) bar->addFilterOption(unit, unit);

    bar->setCurrentFilterText(currentSelection);
    bar->blockSignals(wasBlocked);
}

void SignalsPage::resetFilters()
{
    if (m_tableWidget && m_tableWidget->filterBar())
    {
        const bool wasBlocked = m_tableWidget->filterBar()->blockSignals(true);

        m_tableWidget->filterBar()->setSearchText("");
        m_tableWidget->filterBar()->setCurrentFilterIndex(0); // Select "All"

        m_tableWidget->filterBar()->blockSignals(wasBlocked);
    }
}

// ============================================================================
// UI Setup
// ============================================================================
void SignalsPage::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- Header Card ---------------------------------------------------------
    auto* card = new Core::CardWidget(Constants::SignalsPage::PageHeaderTitle,
                                      Constants::SignalsPage::PageHeaderSubtitle, "", this);
    mainLayout->addWidget(card);

    // --- Searchable Table ----------------------------------------------------
    m_tableWidget = new Core::SearchableFilterTable(this);
    m_tableWidget->setSearchPlaceholder(Constants::SignalsPage::SearchbarText);

    m_tableWidget->configureTableBasics();  // setup table basics
    const auto cardLayout = card->layout();
    cardLayout->addWidget(m_tableWidget);

    auto* table = m_tableWidget->tableView();
    table->setItemDelegate(new SignalTableDelegate(table));
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);

    // Empty label
    m_emptyLabel = new QLabel(Constants::SignalsPage::EmptyLabelText, m_tableWidget);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_emptyLabel->hide();
    m_emptyLabel->setStyleSheet(Style::Common::emptyLabel());

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_tableWidget->layout()))
        layout->addWidget(m_emptyLabel, 1);

    cardLayout->addWidget(m_tableWidget);

    // --- Signal Forwarding ---------------------------------------------------
    connect(m_tableWidget, &Core::SearchableFilterTable::filterTextChanged, this,
            &SignalsPage::filterTextChanged);
    connect(m_tableWidget, &Core::SearchableFilterTable::filterIndexChanged, this,
            &SignalsPage::onFilterIndexChanged);

    // --- Vertical Scrollbar Setup -------------------------------------------
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}
void SignalsPage::updateEmptyState()
{
    if (!m_tableWidget || !m_emptyLabel) return;

    QTableView* view = m_tableWidget->tableView();
    if (!view || !view->model()) return;

    bool isEmpty = (view->model()->rowCount() == 0);

    view->setVisible(!isEmpty);
    m_emptyLabel->setVisible(isEmpty);
}

// ============================================================================
// Column Configuration
// ============================================================================
void SignalsPage::configureColumns(QTableView* table, const QAbstractItemModel* model)
{
    if (!model || model->columnCount() == 0) return;

    const auto& spacing = THEME.spacing();
    auto* header = table->horizontalHeader();

    // Hide columns safely
    auto hideIfExists = [&](int column) {
        if (model->columnCount() > column) table->setColumnHidden(column, true);
    };

    hideIfExists(Constants::Columns::SigMax);
    hideIfExists(Constants::Columns::SigValueType);
    hideIfExists(Constants::Columns::SigReceivers);

    // Set fixed widths
    table->setColumnWidth(Constants::Columns::SigStartBit, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigUnit, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigLength, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigMin, spacing.WidthXs);
    table->setColumnWidth(Constants::Columns::SigFactor, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigOffset, spacing.WidthXs / 2);
    table->setColumnWidth(Constants::Columns::SigByteOrder, spacing.WidthXs);
    table->setColumnWidth(Constants::Columns::SigValueType, spacing.WidthXs / 2);

    // Stretch important columns
    header->setSectionResizeMode(Constants::Columns::SigName, QHeaderView::Stretch);
    header->setSectionResizeMode(Constants::Columns::SigMessage, QHeaderView::Stretch);
}

// ============================================================================
// Apply Styles (refreshable)
// ============================================================================
void SignalsPage::applyStyle()
{
    if (!m_tableWidget) return;

    m_tableWidget->applyStyle();  // refresh table styles
    auto* table = m_tableWidget->tableView();
    if (table && table->verticalScrollBar())
    {
        table->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());
    }

    update();
}

// ============================================================================
// Slots
// ============================================================================
void SignalsPage::onFilterIndexChanged(int)
{
    const QString unit = m_tableWidget->filterBar()->currentFilterData().toString();
    emit filterUnitChanged(unit);
}

auto SignalsPage::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

}  // namespace DbcFile