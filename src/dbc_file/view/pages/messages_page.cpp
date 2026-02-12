#include "messages_page.hpp"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTableView>
#include <QVariant>

// Project Includes
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/message_detail_delegate.hpp"
#include "dbc_file/delegate/message_table_delegate.hpp"
#include "dbc_file/model/dbc_roles.hpp"

namespace DbcFile {

namespace {

// Qt 5 compatible "siblingAtColumn" replacement.
inline auto siblingAtColumnQt5(const QModelIndex& idx, int column) -> QModelIndex
{
    return idx.sibling(idx.row(), column);
}

inline auto formatMessageIdHex(uint id) -> QString
{
    return QStringLiteral("0x%1").arg(id, 0, 16).toUpper();
}

}  // namespace

// =============================================================================
// MessageDetailView
// =============================================================================

MessageDetailView::MessageDetailView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

auto MessageDetailView::getSignalList() const -> QListView*
{
    return m_signalList;
}

void MessageDetailView::setRootIndex(const QModelIndex& index)
{
    // Qt trick: the view treats 'index' as the root and shows only its direct children.
    m_signalList->setRootIndex(index);
}

void MessageDetailView::updateHeaderInfo(const QString& name, uint id, const QString& sender,
                                         int dlc)
{
    if (!m_card) return;

    if (name.isEmpty())
    {
        m_card->setTitle(Constants::MessagesPage::DetailTitlePlaceholder);
        m_card->setSubtitle(Constants::MessagesPage::DetailSubtitlePlaceholder);
        return;
    }

    m_card->setTitle(QString(Constants::MessagesPage::DetailTitle).arg(name));

    const QString subtitle = QString(Constants::MessagesPage::DetailSubtitle)
                                 .arg(formatMessageIdHex(id), sender, QString::number(dlc));
    m_card->setSubtitle(subtitle);
}

void MessageDetailView::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_card =
        new Core::CardWidget(Constants::MessagesPage::DetailTitlePlaceholder,
                             Constants::MessagesPage::DetailSubtitlePlaceholder, QString(), this);

    auto* cardContent = m_card->contentLayout();

    m_signalList = new QListView(m_card);
    m_signalList->setFrameShape(QFrame::NoFrame);
    m_signalList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_signalList->setSelectionMode(QAbstractItemView::NoSelection);
    m_signalList->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    m_signalList->setResizeMode(QListView::Adjust);
    m_signalList->setViewMode(QListView::ListMode);
    m_signalList->setItemDelegate(new MessagesDetailDelegate(m_signalList));

    cardContent->addWidget(m_signalList);
    layout->addWidget(m_card);
}

// =============================================================================
// MessagesPage
// =============================================================================

MessagesPage::MessagesPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void MessagesPage::setupUi()
{
    const auto& spacing = THEME.spacing();
    const int padding = spacing.spacingMd;

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(padding, padding, padding, padding);

    // Master/detail layout: messages table on top, detail pane below.
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setHandleWidth(padding);

    // --- Master: Table card ---
    auto* messageTableCard =
        new Core::CardWidget(Constants::MessagesPage::PageHeaderTitle,
                             Constants::MessagesPage::PageHeaderSubtitle, QString(), this);

    m_messagesTable = new Core::SearchableFilterTable(this);
    configureMasterTable();
    messageTableCard->layout()->addWidget(m_messagesTable);

    // Table filter signals.
    connect(m_messagesTable, &Core::SearchableFilterTable::filterTextChanged, this,
            &MessagesPage::masterFilterTextChanged);

    connect(m_messagesTable, &Core::SearchableFilterTable::filterIndexChanged, this,
            &MessagesPage::onFilterIndexChanged);

    m_splitter->addWidget(messageTableCard);

    // --- Detail: Signal list card ---
    m_detailView = new MessageDetailView(this);
    m_detailView->setVisible(false);

    m_splitter->addWidget(m_detailView);
    m_splitter->setSizes(QList<int>{1000, 0});

    mainLayout->addWidget(m_splitter);
}

void MessagesPage::configureMasterTable()
{
    auto* table = m_messagesTable->tableView();
    table->setItemDelegate(new MessageTableDelegate(table));
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    m_messagesTable->setSearchPlaceholder(Constants::MessagesPage::SearchbarText);
    m_messagesTable->configureTableBasics();
    m_messagesTable->applyTableStyle();
    m_messagesTable->configureHeaderStyle();
}

void MessagesPage::setMasterModel(QAbstractItemModel* model)
{
    if (!m_messagesTable) return;

    auto* table = m_messagesTable->tableView();
    if (!table) return;

    table->setModel(model);
    configureMasterColumns(table, model);

    // Re-wire selection model (it changes whenever the model is replaced).
    if (table->selectionModel())
    {
        disconnect(table->selectionModel(), nullptr, this, nullptr);
        connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                &MessagesPage::onSelectionChanged);
    }
}

void MessagesPage::configureMasterColumns(QTableView* table, const QAbstractItemModel* model)
{
    if (!table || !model || model->columnCount() == 0) return;

    const auto& spacing = THEME.spacing();

    // Show only the message columns.
    for (int i = 0; i < model->columnCount(); ++i)
    {
        table->setColumnHidden(i, i >= Constants::Columns::MsgColumnCount);
    }

    auto* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(false);

    header->setSectionResizeMode(Constants::Columns::MsgName, QHeaderView::Stretch);

    header->setSectionResizeMode(Constants::Columns::MsgId, QHeaderView::Fixed);
    table->setColumnWidth(Constants::Columns::MsgId, spacing.WidthSm);

    header->setSectionResizeMode(Constants::Columns::MsgDlc, QHeaderView::Fixed);
    table->setColumnWidth(Constants::Columns::MsgDlc, spacing.WidthXs);

    header->setSectionResizeMode(Constants::Columns::MsgSender, QHeaderView::Interactive);
    table->setColumnWidth(Constants::Columns::MsgSender, spacing.WidthSm);

    header->setSectionResizeMode(Constants::Columns::MsgSigCount, QHeaderView::Fixed);
    table->setColumnWidth(Constants::Columns::MsgSigCount, spacing.WidthXs);
}

void MessagesPage::setDetailModel(QAbstractItemModel* model)
{
    if (!m_detailView) return;

    if (auto* list = m_detailView->getSignalList())
    {
        list->setModel(model);
    }
}

void MessagesPage::selectMessageIndex(const QModelIndex& index)
{
    if (!m_detailView || !index.isValid()) return;

    const QModelIndex nameIdx = siblingAtColumnQt5(index, Constants::Columns::MsgName);
    const QModelIndex idIdx = siblingAtColumnQt5(index, Constants::Columns::MsgId);
    const QModelIndex senderIdx = siblingAtColumnQt5(index, Constants::Columns::MsgSender);
    const QModelIndex dlcIdx = siblingAtColumnQt5(index, Constants::Columns::MsgDlc);

    const QString name = nameIdx.data(Qt::DisplayRole).toString();

    uint id = idIdx.data(DbcRoles::Role_Id).toUInt();
    if (id == 0)
    {
        id = idIdx.data(Qt::DisplayRole).toUInt();
    }

    const QString sender = senderIdx.data(Qt::DisplayRole).toString();
    const int dlc = dlcIdx.data(Qt::DisplayRole).toInt();

    m_detailView->updateHeaderInfo(name, id, sender, dlc);
    m_detailView->setRootIndex(index);
}

void MessagesPage::onSelectionChanged(const QModelIndex& current, const QModelIndex&)
{
    if (!current.isValid())
    {
        m_detailView->setVisible(false);
        return;
    }

    if (m_detailView->isHidden())
    {
        m_detailView->setVisible(true);
        m_splitter->setSizes(QList<int>{500, 500});
        m_splitter->setStretchFactor(0, 1);
        m_splitter->setStretchFactor(1, 1);
    }

    emit messageSelectionChanged(current);
}

void MessagesPage::onFilterIndexChanged(int index)
{
    if (index < 0 || !m_messagesTable) return;

    const QString sender = m_messagesTable->filterBar()->currentFilterData().toString();
    emit filterSenderChanged(sender);
}

auto MessagesPage::setAvailableSenders(const QStringList& senders) -> void
{
    if (!m_messagesTable) return;

    auto* bar = m_messagesTable->filterBar();
    if (!bar) return;

    const bool wasBlocked = bar->blockSignals(true);
    const QString currentSelection = bar->currentFilterText();

    bar->clearFilterOptions();
    bar->addFilterOption(Constants::MessagesPage::FilterAllText, QString());

    for (const auto& sender : senders)
    {
        bar->addFilterOption(sender, sender);
    }

    bar->setCurrentFilterText(currentSelection);
    bar->blockSignals(wasBlocked);
}

}  // namespace DbcFile
