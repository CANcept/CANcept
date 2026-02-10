#include "signals_page.hpp"

#include <qstandarditemmodel.h>

#include <QHeaderView>

#include "core/enum/dbc_itemtype.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/signal_table_delegate.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "qwt_picker_machine.h"
namespace Core {
enum class DbcItemType;
}
namespace DbcFile {
// --- SignalsPage Dummy ---
SignalsPage::SignalsPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void SignalsPage::populateUnitFilter(const QAbstractItemModel* model) const
{
    if (!model) return;

    auto* filterBar = m_tableWidget->filterBar();
    // 1. collect units
    QSet<QString> units;

    const int rows = model->rowCount();

    for (int i = 0; i < rows; ++i) {
        QModelIndex idx = model->index(i, 0);
        QString unit = model->data(idx, DbcRoles::Role_Unit).toString();
        if (!unit.isEmpty()) {
            units.insert(unit);
        }
    }

    // 2. Sort units
    QStringList sortedUnits = units.values();
    sortedUnits.sort(Qt::CaseInsensitive);

    // 3. prepend default option
    QStringList options;
    options << Constants::SignalsPage::FilterAllText;
    options.append(sortedUnits);

    // 4. Fill into filterBar
    QString currentSelection = filterBar->currentFilter();

    filterBar->setFilterOptions(options);

    if (!currentSelection.isEmpty() && options.contains(currentSelection)) {
        filterBar->setCurrentFilter(currentSelection);
    } else {
        filterBar->setCurrentFilterIndex(0);
    }
}

void SignalsPage::setModel(QAbstractItemModel* model) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();
    if (m_tableWidget && m_tableWidget->tableView())
    {
        auto* table = m_tableWidget->tableView();

        // Set delegate and model
        table->setItemDelegate(new SignalTableDelegate(table));
        table->setModel(model);
        populateUnitFilter(model);

        // Layout Basics
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        table->setStyleSheet(QString(
            "QTableView {"
            "border: none"
            // "   border: %1px solid %2;"
            "  border-radius: %3px;"
            "   background-color: %4;"
            "}"
            )
            .arg(spacing.borderThin)
            .arg(colors.borderSubtle.name(QColor::HexArgb))
            .arg(spacing.radiusSm)
            .arg(colors.surfaceMain.name())
            );

        // Header Safety
        auto* hHeader = table->horizontalHeader();

        if (model && model->columnCount() > 0)
        {
            // Hide colums
            if (model->columnCount() > Constants::Columns::SigMax)
                table->setColumnHidden(Constants::Columns::SigMax, true);
            if (model->columnCount() > Constants::Columns::SigValueType)
                table->setColumnHidden(Constants::Columns::SigValueType, true);
            if (model->columnCount() > Constants::Columns::SigReceivers)
                table->setColumnHidden(Constants::Columns::SigReceivers, true);

            hHeader->setSectionResizeMode(QHeaderView::Interactive);
            hHeader->setStretchLastSection(false);

            table->setColumnWidth(Constants::Columns::SigStartBit, spacing.WidthXs / 2);
            table->setColumnWidth(Constants::Columns::SigUnit,     spacing.WidthXs / 2);
            table->setColumnWidth(Constants::Columns::SigLength,   spacing.WidthXs / 2);
            table->setColumnWidth(Constants::Columns::SigMin,      spacing.WidthXs);
            table->setColumnWidth(Constants::Columns::SigFactor,   spacing.WidthXs / 2);
            table->setColumnWidth(Constants::Columns::SigOffset, spacing.WidthXs / 2);
            table->setColumnWidth(Constants::Columns::SigByteOrder, spacing.WidthXs);
            table->setColumnWidth(Constants::Columns::SigValueType, spacing.WidthXs / 2);

            hHeader->setSectionResizeMode(Constants::Columns::SigName, QHeaderView::Stretch);
            hHeader->setSectionResizeMode(Constants::Columns::SigMessage, QHeaderView::Stretch);

            // Header Styling
            hHeader->setStyleSheet(QString(
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

        // Hide vertical header
        if (table->verticalHeader()) {
            table->verticalHeader()->hide();
        }

        table->setShowGrid(false);
        table->setAlternatingRowColors(false);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
}
void SignalsPage::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    // --- Main Layout ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- Header Card ---
    auto* card = new Core::CardWidget(Constants::SignalsPage::PageHeaderTitle,
                                      Constants::SignalsPage::PageHeaderSubtitle, "", this);
    mainLayout->addWidget(card);
    auto* cardLayout = card->layout();

    // --- Search & Filter Table ---
    m_tableWidget = new Core::SearchableFilterTable(this);

    // Configure search bar
    m_tableWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);

    // Add table widget to card layout
    cardLayout->addWidget(m_tableWidget);

    // --- Connect signals for forwarding ---
    connect(m_tableWidget, &Core::SearchableFilterTable::filterTextChanged, this,
            &SignalsPage::filterTextChanged);

}

}  // namespace DbcFile