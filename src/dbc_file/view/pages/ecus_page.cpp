#include "ecus_page.hpp"

#include <QHeaderView>

#include "core/constants.hpp"
#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/common/searchable_filter_widgets.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/ecu_tree_delegate.hpp"

namespace DbcFile {

EcusPage::EcusPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void EcusPage::setModel(QAbstractItemModel* model) const
{
    if (m_treeWidget && model)
    {
        m_treeWidget->treeView()->setModel(model);
        m_treeWidget->treeView()->expandAll();
    }
}

void EcusPage::setupUi()
{
    const auto& spacing = THEME.spacing();

    // --- Main Layout ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- Header Card ---
    auto* card = new Core::CardWidget(Constants::EcusPage::PageHeaderTitle,
                                      Constants::EcusPage::PageHeaderSubtitle, "", this);
    mainLayout->addWidget(card);
    auto* cardLayout = card->layout();

    // --- Search & Filter Tree ---
    m_treeWidget = new Core::SearchableFilterTree(this);

    // Configure search bar
    m_treeWidget->setSearchPlaceholder(Constants::EcusPage::SearchbarText);

    // Configure filter options
    QStringList options = {Constants::EcusPage::FilterAllText, Constants::EcusPage::FilterActive};
    m_treeWidget->setFilterOptions(options);

    // --- TreeView setup ---
    QTreeView* view = m_treeWidget->treeView();
    if (view)
    {
        // Assign custom delegate
        auto* delegate = new EcuTreeDelegate(view, this);
        view->setItemDelegate(delegate);

        // Visual setup for card/tree style
        view->setHeaderHidden(true);
        view->setAnimated(true);
        view->setSelectionMode(QAbstractItemView::NoSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->header()->setSectionsClickable(false);
        view->setExpandsOnDoubleClick(false);
        view->setItemsExpandable(true);

        // Custom branch icons & style
        QString style =
            QString(R"(
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
                .arg(Core::Constants::ARROW_RIGHT_ICON, Core::Constants::ARROW_DOWN_ICON);

        view->setRootIsDecorated(true);
        view->setStyleSheet(style);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // Add tree widget to card layout
    cardLayout->addWidget(m_treeWidget);

    // --- Connect signals for forwarding ---
    connect(m_treeWidget, &Core::SearchableFilterTree::filterTextChanged, this,
            &EcusPage::filterTextChanged);

    connect(m_treeWidget, &Core::SearchableFilterTree::filterIndexChanged, this,
            &EcusPage::filterIndexChanged);
}

}  // namespace DbcFile