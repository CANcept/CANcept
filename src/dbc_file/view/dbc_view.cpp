#include "dbc_view.hpp"

#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/enum/dbc_itemtype.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/sidebar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/ecu_tree_delegate.hpp"
#include "dbc_file/view/proxies/ecu_tree_proxy.hpp"
#include "dbc_file/view/proxies/flat_list_proxy.hpp"
#include "dbc_file/view/proxies/single_message_proxy.hpp"

namespace DbcFile {
DbcView::DbcView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
DbcView::~DbcView() = default;
auto DbcView::getLoadPage() const -> LoadPage&
{
    return *m_loadPage;
}
void DbcView::setSourceModel(QAbstractItemModel* model)
{
    if (!model) return;

    m_model = model;

    // Create proxies + connect to source model
    m_ecuOverviewProxy = std::make_unique<FlatListProxy>(Core::DbcItemType::Ecu, this);
    m_ecuOverviewProxy->setSourceModel(model);
    m_ecuOverviewProxy->rebuildMapping();

    m_messagesProxy = std::make_unique<FlatListProxy>(Core::DbcItemType::Message, this);
    m_messagesProxy->setSourceModel(model);
    m_messagesProxy->rebuildMapping();

    // Pass proxies to overview page lists
    m_overviewPage->getEcuList()->setModel(m_ecuOverviewProxy.get());
    m_overviewPage->getMessageList()->setModel(m_messagesProxy.get());

    m_ecuTreeProxy = std::make_unique<EcuTreeProxy>(this);
    m_ecuTreeProxy->setSourceModel(model);
    m_ecuPage->setModel(m_ecuTreeProxy.get());

    // Update Labels in m_overviewPage at model reset
    connect(model, &QAbstractItemModel::modelReset, this,
            [this, model]() { m_overviewPage->updateLabels(model); });
}
void DbcView::setDataItemDelegate(QAbstractItemDelegate* delegate) {}
void DbcView::setNavigationEnabled(const bool enabled) const
{
    m_sidebar->setNavigationEnabled(enabled);
}
void DbcView::onSidebarSelectionChanged(int index) const
{
    // Guard against invalid indices
    if (index < 0 || index >= m_contentStack->count()) return;

    // Avoid redundant page switches
    if (m_contentStack->currentIndex() == index)
    {
        return;
    }

    // Load page cleanup if leaving load page
    if (m_contentStack->currentIndex() == 0 && index != 0)
    {
        m_loadPage->resetStatus();
    }

    // Switch page in content stack
    m_contentStack->setCurrentIndex(index);
}

void DbcView::onEcuFilterTextChanged(const QString& text) const
{
    m_ecuTreeProxy->setSearchText(text);
}
void DbcView::onEcuFilterIndexChanged(int index) const
{
    m_ecuTreeProxy->setFilterCategory(index);
}
void DbcView::onMessageFilterTextChanged(const QString& text) {}
void DbcView::onMessageFilterTypeChanged(int index) {}
void DbcView::onMessageSelected(const QModelIndex& proxyIndex) {}
void DbcView::onSignalFilterTextChanged(const QString& text) {}
void DbcView::onSignalFilterTypeChanged(int index) {}

void DbcView::addPage(QWidget* page, const QString& title, const QString& iconPath,
                      const bool enabled) const
{
    m_contentStack->addWidget(page);
    m_sidebar->addTab(QIcon(iconPath), title, enabled);
}
void DbcView::setupSidebarTabs()
{
    // 1. Load New
    m_loadPage = new LoadPage(this);
    addPage(m_loadPage, Constants::Sidebar::TitleLoadNew, Constants::Sidebar::IconLoadNew, true);

    // 2. Overview
    m_overviewPage = new OverviewPage(this);
    addPage(m_overviewPage, Constants::Sidebar::TitleOverview, Constants::Sidebar::IconOverview,
            false);

    // 3. ECUs
    m_ecuPage = new EcusPage(this);
    addPage(m_ecuPage, Constants::Sidebar::TitleEcus, Constants::Sidebar::IconEcus, false);

    // 4. Messages
    m_messagesPage = new MessagesPage(this);
    addPage(m_messagesPage, Constants::Sidebar::TitleMessages, Constants::Sidebar::IconMessages,
            false);

    // 5. Messages
    m_signalsPage = new SignalsPage(this);
    addPage(m_signalsPage, Constants::Sidebar::TitleSignals, Constants::Sidebar::IconSignals,
            false);
}
void DbcView::setupUi()
{
    // --- Main layout setup ---
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Sidebar setup ---
    m_sidebar = new Core::Sidebar(this);
    m_sidebar->setToolTipText(
        Constants::Sidebar::HoverText);  // Tooltip shown for disabled navigation entries
    mainLayout->addWidget(m_sidebar);
    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack);
    setupSidebarTabs();

    // --- Signal connections ---
    setupConnections();
}

void DbcView::setupConnections()
{
    connect(m_sidebar, &Core::Sidebar::tabSelected, this,
            &DbcFile::DbcView::onSidebarSelectionChanged);
    connect(m_loadPage, &LoadPage::fileSelected, this,
            [this](const QString& path) { emit fileLoadRequested(path); });

    connect(m_ecuPage, &EcusPage::filterTextChanged, this, &DbcView::onEcuFilterTextChanged);
    connect(m_ecuPage, &EcusPage::filterIndexChanged, this, &DbcView::onEcuFilterIndexChanged);
}
}  // namespace DbcFile