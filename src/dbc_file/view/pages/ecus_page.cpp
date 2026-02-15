#include "ecus_page.hpp"

#include <QHeaderView>
#include <QScrollBar>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/ecu_tree_delegate.hpp"
#include "dbc_file/styles.hpp"

namespace DbcFile {

// ============================================================================
// Constructor
// ============================================================================

EcusPage::EcusPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyle();
}

// ============================================================================
// Public API
// ============================================================================

void EcusPage::setModel(QAbstractItemModel* model)
{
    if (!m_treeWidget || !model) return;

    QTreeView* view = m_treeWidget->treeView();
    if (!view) return;

    view->setModel(model);
    view->expandAll();

    // disconnect old signals
    disconnect(model, nullptr, this, nullptr);

    // Connect model changes to update empty state
    connect(model, &QAbstractItemModel::modelReset, this, &EcusPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsInserted, this, &EcusPage::updateEmptyState);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &EcusPage::updateEmptyState);

    updateEmptyState();
}

// ============================================================================
// UI Setup
// ============================================================================

void EcusPage::setupUi()
{
    createLayout();
    createHeaderCard();
    createTreeSection();
    connectSignals();
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

    // Setup search/filter
    m_treeWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);
    m_treeWidget->setFilterOptions(
        {Constants::EcusPage::FilterAllText, Constants::EcusPage::FilterActive});

    // Empty label
    m_emptyLabel = new QLabel(Constants::EcusPage::EmptyLabelText, m_treeWidget);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_emptyLabel->hide();
    m_emptyLabel->setStyleSheet(Style::Common::emptyLabel());

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_treeWidget->layout()))
        layout->addWidget(m_emptyLabel, 1);

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
    view->setRootIsDecorated(true);

    // Scrollbars: vertical always on, horizontal off
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    view->installEventFilter(this);
}

// ============================================================================
// Styling
// ============================================================================

void EcusPage::applyStyle()
{
    if (!m_treeWidget) return;

    QTreeView* view = m_treeWidget->treeView();
    if (!view) return;

    // Apply tree style
    view->setStyleSheet(Style::EcusPage::treeStyle());

    // Apply vertical scrollbar style
    if (view->verticalScrollBar())
        view->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());

    update();
}

// ============================================================================
// Signals
// ============================================================================

void EcusPage::connectSignals()
{
    if (!m_treeWidget) return;

    connect(m_treeWidget, &Core::SearchableFilterTree::filterTextChanged, this,
            &EcusPage::filterTextChanged);

    connect(m_treeWidget, &Core::SearchableFilterTree::filterIndexChanged, this,
            &EcusPage::filterIndexChanged);
}

// ============================================================================
// Empty State Handling
// ============================================================================

void EcusPage::updateEmptyState()
{
    if (!m_treeWidget || !m_emptyLabel) return;

    QTreeView* view = m_treeWidget->treeView();
    if (!view || !view->model()) return;

    bool isEmpty = (view->model()->rowCount() == 0);

    view->setVisible(!isEmpty);
    m_emptyLabel->setVisible(isEmpty);
}

auto EcusPage::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }

    return QWidget::event(event);
}

}  // namespace DbcFile