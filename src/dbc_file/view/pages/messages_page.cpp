#include "messages_page.hpp"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTableView>
#include <QVariant>

// Project Includes
#include <qevent.h>
#include <qscrollbar.h>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/util/dbc_utils.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "core/widgets/common/styled_filter_bar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/message_detail_delegate.hpp"
#include "dbc_file/delegate/message_table_delegate.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/styles.hpp"

namespace DbcFile {

// =============================================================================
// MessageDetailView
// =============================================================================

MessageDetailView::MessageDetailView(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyle();
}

auto MessageDetailView::getSignalList() const -> QListView*
{
    return m_signalList;
}

void MessageDetailView::setRootIndex(const QModelIndex& index)
{
    m_signalList->setRootIndex(index);

    int signalCount = 0;
    if (index.isValid() && m_signalList->model())
    {
        signalCount = m_signalList->model()->rowCount(index);
    }

    if (signalCount > 0)
    {
        m_stack->setCurrentIndex(0);
    } else
    {
        m_stack->setCurrentIndex(1);
    }
}
void MessageDetailView::applyStyle()
{
    if (!m_detailLabel) return;

    // Apply stylesheet provided by styles.cpp
    m_detailLabel->setStyleSheet(Style::Common::emptyLabel());

    update();
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
                                 .arg(Core::Util::formatId(id), sender, QString::number(dlc));
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

    // Create stack
    m_stack = new QStackedWidget(m_card);

    // Create List
    m_signalList = new QListView(m_stack);
    m_signalList->setFrameShape(QFrame::NoFrame);
    m_signalList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_signalList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_signalList->setSelectionMode(QAbstractItemView::NoSelection);
    m_signalList->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    m_signalList->setResizeMode(QListView::Adjust);
    m_signalList->setViewMode(QListView::ListMode);
    m_signalList->setItemDelegate(new MessagesDetailDelegate(m_signalList));

    // Add list as page 0
    m_stack->addWidget(m_signalList);

    // Create label
    m_detailLabel = new QLabel(Constants::MessagesPage::NoSignalsIndicator, m_stack);
    m_detailLabel->setAlignment(Qt::AlignCenter);

    // Add label as page 1
    m_stack->addWidget(m_detailLabel);

    // Add stack to card
    cardContent->addWidget(m_stack);

    layout->addWidget(m_card);
}
auto MessageDetailView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

// =============================================================================
// MessagesPage
// =============================================================================

MessagesPage::MessagesPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyle();
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

    // Empty label
    m_emptyLabel = new QLabel(Constants::MessagesPage::EmptyLabelText, m_messagesTable);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_emptyLabel->hide();
    m_emptyLabel->setStyleSheet(Style::Common::emptyLabel());

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_messagesTable->layout()))
        layout->addWidget(m_emptyLabel, 1);

    messageTableCard->layout()->addWidget(m_messagesTable);

    if (auto* table = m_messagesTable->tableView())
    {
        table->viewport()->installEventFilter(this);
    }
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
}
void MessagesPage::applyStyle()
{
    // --- Style Master Table ---
    if (auto* table = m_messagesTable->tableView())
        table->verticalScrollBar()->setStyleSheet(DbcFile::Style::Common::verticalScrollBar());

    // --- Style Detail View ---
    if (m_detailView && m_detailView->getSignalList())
        m_detailView->getSignalList()->verticalScrollBar()->setStyleSheet(
            DbcFile::Style::Common::verticalScrollBar());

    // --- Forward style refresh to child widgets ---
    if (m_messagesTable) m_messagesTable->applyStyle();
    if (m_detailView) m_detailView->applyStyle();

    update();
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

    connect(model, &QAbstractItemModel::modelReset, this, &MessagesPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsInserted, this, &MessagesPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &MessagesPage::updateEmptyState);
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

    const QModelIndex nameIdx = Core::Util::siblingAtColumnQt5(index, Constants::Columns::MsgName);
    const QModelIndex idIdx = Core::Util::siblingAtColumnQt5(index, Constants::Columns::MsgId);
    const QModelIndex senderIdx =
        Core::Util::siblingAtColumnQt5(index, Constants::Columns::MsgSender);
    const QModelIndex dlcIdx = Core::Util::siblingAtColumnQt5(index, Constants::Columns::MsgDlc);

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
void MessagesPage::mousePressEvent(QMouseEvent* event)
{
    // Click out of table -> deselect
    if (m_messagesTable && m_messagesTable->tableView())
    {
        m_messagesTable->tableView()->clearSelection();
        m_messagesTable->tableView()->setCurrentIndex(QModelIndex());
    }

    // Standard handling
    QWidget::mousePressEvent(event);
}
auto MessagesPage::eventFilter(QObject* watched, QEvent* event) -> bool
{
    auto* table = m_messagesTable->tableView();

    // only check clicks within table viewport
    if (table && watched == table->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = table->indexAt(mouseEvent->pos());

        if (index.isValid())
        {
            // Case A: click on row -> check selection model: is row already selected?
            if (table->selectionModel()->isSelected(index))
            {
                table->clearSelection();
                table->setCurrentIndex(QModelIndex());
                return true;
            }
        } else
        {
            // Case B: Click in white space below rows
            table->clearSelection();
            table->setCurrentIndex(QModelIndex());
        }
    }

    return QWidget::eventFilter(watched, event);
}

auto MessagesPage::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

void MessagesPage::updateEmptyState()
{
    if (!m_messagesTable || !m_emptyLabel) return;

    QTableView* view = m_messagesTable->tableView();
    if (!view || !view->model()) return;

    bool isEmpty = (view->model()->rowCount() == 0);

    view->setVisible(!isEmpty);
    m_emptyLabel->setVisible(isEmpty);
}

}  // namespace DbcFile
