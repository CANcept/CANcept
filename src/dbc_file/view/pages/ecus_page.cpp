#include "ecus_page.hpp"

#include <QHeaderView>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/ecu_tree_delegate.hpp"
#include "dbc_file/styles.hpp"

namespace DbcFile {

EcusPage::EcusPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void EcusPage::setModel(QAbstractItemModel* model)
{
    if (!m_treeWidget || !model) return;

    if (auto* view = m_treeWidget->treeView())
    {
        view->setModel(model);
        view->expandAll();

        // delete old connections
        disconnect(model, nullptr, this, nullptr);

        // Connect to show label when filtered
        connect(model, &QAbstractItemModel::modelReset, this, &EcusPage::updateEmptyState);
        connect(model, &QAbstractItemModel::rowsInserted, this, &EcusPage::updateEmptyState);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &EcusPage::updateEmptyState);

        // Set initial state
        updateEmptyState();
    }
}

void EcusPage::updateEmptyState()
{
    if (!m_treeWidget || !m_emptyLabel) return;

    auto* view = m_treeWidget->treeView();
    if (!view || !view->model()) return;

    // Check vor rows in tree
    bool isEmpty = (view->model()->rowCount() == 0);

    if (isEmpty) {
        view->hide();
        m_emptyLabel->show();
    } else {
        view->show();
        m_emptyLabel->hide();
    }
}

void EcusPage::createLayout()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    setLayout(mainLayout);
}
void EcusPage::createHeaderCard()
{
    auto* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) return;

    auto* card = new Core::CardWidget(Constants::EcusPage::PageHeaderTitle,
                                      Constants::EcusPage::PageHeaderSubtitle, QString(), this);

    mainLayout->addWidget(card);

    m_cardLayout = card->layout();
}
void EcusPage::createTreeSection()
{
    m_treeWidget = new Core::SearchableFilterTree(this);

    m_treeWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);
    m_treeWidget->setFilterOptions(
        {Constants::EcusPage::FilterAllText, Constants::EcusPage::FilterActive});


    m_emptyLabel = new QLabel(Constants::EcusPage::EmptyLabelText, m_treeWidget);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->hide(); // hide label
    m_emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_emptyLabel->setStyleSheet(Style::EcusPage::emptyLabel());

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_treeWidget->layout())) {
        layout->addWidget(m_emptyLabel, 1);
    }

    if (m_cardLayout) m_cardLayout->addWidget(m_treeWidget);

    configureTreeView();
}
void EcusPage::configureTreeView()
{
    if (!m_treeWidget) return;

    QTreeView* view = m_treeWidget->treeView();
    if (!view) return;

    view->setItemDelegate(new EcuTreeDelegate(view, this));

    view->setHeaderHidden(true);
    view->setAnimated(true);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setExpandsOnDoubleClick(false);
    view->setItemsExpandable(true);
    view->header()->setSectionsClickable(false);
    view->header()->setSectionResizeMode(QHeaderView::Stretch);
    view->header()->setStretchLastSection(false);
    view->setWordWrap(true);

    applyTreeStyle(view);

    view->setRootIsDecorated(true);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->installEventFilter(this);
}
void EcusPage::applyTreeStyle(QTreeView* view)
{
    view->setStyleSheet(Style::EcusPage::treeStyle());
}
void EcusPage::connectSignals()
{
    if (!m_treeWidget) return;

    connect(m_treeWidget, &Core::SearchableFilterTree::filterTextChanged, this,
            &EcusPage::filterTextChanged);

    connect(m_treeWidget, &Core::SearchableFilterTree::filterIndexChanged, this,
            &EcusPage::filterIndexChanged);
}
void EcusPage::setupUi()
{
    createLayout();
    createHeaderCard();
    createTreeSection();
    connectSignals();
}
}  // namespace DbcFile