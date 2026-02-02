//
// Created by Adrian Rupp on 20.01.26.
#include "dbc_view.hpp"

#include <QList>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/sidebar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/page_delegates.hpp"

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

    // Update overview labels ind overviewpage and connect it to modelreset
    updateOverviewLabels();
    connect(model, &QAbstractItemModel::modelReset, this, [this]() { updateOverviewLabels(); });
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
void DbcView::updateOverviewLabels() const
{
    if (!m_overviewPage || !m_model) return;

    // Overview item always first child to root
    if (m_model->rowCount() == 0) return;

    QModelIndex overviewIndex = m_model->index(0, 0, QModelIndex());
    if (!overviewIndex.isValid()) return;

    m_overviewPage->setFileName(
        m_model->data(m_model->index(0, Constants::Columns::OvFilename)).toString());
    m_overviewPage->setVersion(
        m_model->data(m_model->index(0, Constants::Columns::OvVersion)).toString());
    m_overviewPage->setEcuCount(
        m_model->data(m_model->index(0, Constants::Columns::OvEcuCount)).toString());
    m_overviewPage->setMessageCount(
        m_model->data(m_model->index(0, Constants::Columns::OvMsgCount)).toString());
    m_overviewPage->setSignalCount(
        m_model->data(m_model->index(0, Constants::Columns::OvSigCount)).toString());
    m_overviewPage->setOrphanCount(
        m_model->data(m_model->index(0, Constants::Columns::OvOrphans)).toString());
}

void DbcView::onEcuFilterTextChanged(const QString& text) {}
void DbcView::onEcuFilterTypeChanged(int index) {}
void DbcView::onMessageFilterTextChanged(const QString& text) {}
void DbcView::onMessageFilterTypeChanged(int index) {}
void DbcView::onMessageSelected(const QModelIndex& proxyIndex) {}
void DbcView::onSignalFilterTextChanged(const QString& text) {}
void DbcView::onSignalFilterTypeChanged(int index) {}

void DbcView::setupUi()
{
    // Create main layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Sidebar setup ---
    m_sidebar = new Core::Sidebar(this);

    // Add sidebar tabs
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconLoadNew), Constants::Sidebar::TitleLoadNew,
                      true);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconOverview), Constants::Sidebar::TitleOverview,
                      true);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconEcus), Constants::Sidebar::TitleEcus, false);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconMessages), Constants::Sidebar::TitleMessages,
                      false);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconSignals), Constants::Sidebar::TitleSignals,
                      false);

    // Tooltip shown for disabled navigation entries
    m_sidebar->setToolTipText(Constants::Sidebar::HoverText);
    mainLayout->addWidget(m_sidebar);

    // --- Content stack setup ---
    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack);
    createSubViews();

    // --- Signal connections ---
    setupConnections();
}
void DbcFile::DbcView::createSubViews()
{
    m_loadPage = new LoadPage(this);
    m_contentStack->addWidget(m_loadPage);

    m_overviewPage = new OverviewPage(this);
    m_contentStack->addWidget(m_overviewPage);

    m_ecuPage = new EcusPage(this);
    m_contentStack->addWidget(m_ecuPage);

    m_messagesPage = new MessagesPage(this);
    m_contentStack->addWidget(m_messagesPage);

    m_signalsPage = new SignalsPage(this);
    m_contentStack->addWidget(m_signalsPage);
}
void DbcFile::DbcView::setupConnections()
{
    connect(m_sidebar, &Core::Sidebar::tabSelected, this,
            &DbcFile::DbcView::onSidebarSelectionChanged);
    connect(m_loadPage, &LoadPage::fileSelected, this,
            [this](const QString& path) { emit fileLoadRequested(path); });
}
}  // namespace DbcFile