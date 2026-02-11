#include "ecus_page.hpp"

#include <QHeaderView>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/ecu_tree_delegate.hpp"

namespace DbcFile {

EcusPage::EcusPage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void EcusPage::setModel(QAbstractItemModel* model) const
{
    if (!m_treeWidget || !model)
        return;

    if (auto* view = m_treeWidget->treeView())
    {
        view->setModel(model);
        view->expandAll();
    }
}

void EcusPage::createLayout()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd,
                                   spacing.spacingMd,
                                   spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    setLayout(mainLayout);
}
void EcusPage::createHeaderCard()
{
    auto* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout)
        return;

    auto* card = new Core::CardWidget(
        Constants::EcusPage::PageHeaderTitle,
        Constants::EcusPage::PageHeaderSubtitle,
        QString(),
        this);

    mainLayout->addWidget(card);

    m_cardLayout = card->layout();
}
void EcusPage::createTreeSection()
{
    m_treeWidget = new Core::SearchableFilterTree(this);

    m_treeWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);
    m_treeWidget->setFilterOptions(
        {Constants::EcusPage::FilterAllText, Constants::EcusPage::FilterActive});

    if (m_cardLayout) m_cardLayout->addWidget(m_treeWidget);

    configureTreeView();
}
void EcusPage::configureTreeView()
{
    if (!m_treeWidget)
        return;

    QTreeView* view = m_treeWidget->treeView();
    if (!view)
        return;

    view->setItemDelegate(new EcuTreeDelegate(view, this));

    view->setHeaderHidden(true);
    view->setAnimated(true);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setExpandsOnDoubleClick(false);
    view->setItemsExpandable(true);
    view->header()->setSectionsClickable(false);

    applyTreeStyle(view);

    view->setRootIsDecorated(true);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
void EcusPage::applyTreeStyle(QTreeView* view)
{
    const QString style = QString(R"(
        QTreeView {
            background: transparent;
            border: none;
            outline: none;
        }
        QTreeView::item { border: none; }
        QTreeView::branch { width: 18px; height: 18px; }
        QTreeView::branch:closed:has-children { image: url(%1); }
        QTreeView::branch:open:has-children { image: url(%2); }
    )")
        .arg(Core::Constants::ARROW_RIGHT_ICON,
             Core::Constants::ARROW_DOWN_ICON);

    view->setStyleSheet(style);
}
void EcusPage::connectSignals()
{
    if (!m_treeWidget)
        return;

    connect(m_treeWidget,
            &Core::SearchableFilterTree::filterTextChanged,
            this,
            &EcusPage::filterTextChanged);

    connect(m_treeWidget,
            &Core::SearchableFilterTree::filterIndexChanged,
            this,
            &EcusPage::filterIndexChanged);
}
void EcusPage::setupUi()
{
    createLayout();
    createHeaderCard();
    createTreeSection();
    connectSignals();
}

} // namespace DbcFile