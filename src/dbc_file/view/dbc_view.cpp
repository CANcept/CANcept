//
// Created by Adrian Rupp on 20.01.26.
#include "dbc_view.hpp"

#include <QList>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/ui/widgets/sidebar.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/delegate/page_delegates.hpp"

namespace DbcFile {
DbcFile::DbcView::DbcView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
DbcFile::DbcView::~DbcView() = default;
auto DbcView::getLoadPage() const -> LoadPage&
{
    return *m_loadPage;
}
void DbcFile::DbcView::setSourceModel(QAbstractItemModel* model) {}
void DbcFile::DbcView::setDataItemDelegate(QAbstractItemDelegate* delegate) {}
void DbcFile::DbcView::setNavigationEnabled(const bool enabled) const
{
    m_sidebar->setNavigationEnabled(enabled);
}
void DbcFile::DbcView::onSidebarSelectionChanged(int index)
{
    // Guard against invalid indices
    if (index < 0 || index >= m_contentStack->count()) return;

    // Avoid redundant page switches
    if (m_contentStack->currentIndex() == index) {
        return;
    }

    // Load page cleanup if leaving load page
    if (m_contentStack->currentIndex() == 0 && index != 0) {
        m_loadPage->resetStatus();
    }

    // Switch page in content stack
    m_contentStack->setCurrentIndex(index);
}
void DbcFile::DbcView::onEcuFilterTextChanged(const QString& text) {}
void DbcFile::DbcView::onEcuFilterTypeChanged(int index) {}
void DbcFile::DbcView::onMessageFilterTextChanged(const QString& text) {}
void DbcFile::DbcView::onMessageFilterTypeChanged(int index) {}
void DbcFile::DbcView::onMessageSelected(const QModelIndex& proxyIndex) {}
void DbcFile::DbcView::onSignalFilterTextChanged(const QString& text) {}
void DbcFile::DbcView::onSignalFilterTypeChanged(int index) {}


void DbcFile::DbcView::setupUi()
{
    // Create main layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);


    // --- Sidebar setup ---
    m_sidebar = new Core::Sidebar(this);

    // Add sidebar tabs
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconLoadNew), Constants::Sidebar::TitleLoadNew, true);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconOverview), Constants::Sidebar::TitleOverview, false);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconEcus), Constants::Sidebar::TitleEcus,     false);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconMessages), Constants::Sidebar::TitleMessages, false);
    m_sidebar->addTab(QIcon(Constants::Sidebar::IconSignals), Constants::Sidebar::TitleSignals,  false);

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
    connect(m_sidebar, &Core::Sidebar::tabSelected, this, &DbcFile::DbcView::onSidebarSelectionChanged);
    connect(m_loadPage, &LoadPage::fileSelected, this,
            [this](const QString& path) { emit fileLoadRequested(path); });
}
}  // namespace DbcFile