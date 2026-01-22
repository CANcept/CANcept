//
// Created by Adrian Rupp on 20.01.26.
#include "dbc_view.hpp"

#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/constants.hpp"
#include "core/theme/theme_manager.hpp"
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
void DbcFile::DbcView::setNavigationEnabled(bool enabled)
{
    auto* sidebarModel = static_cast<QStandardItemModel*>(m_sidebarList->model()); // auto return QAbstractItemModel
    for (int i = 1; i < sidebarModel->rowCount(); i++)
    {
        if (auto* item = sidebarModel->item(i))
        {
            item->setEnabled(enabled);
        }
    }

    if (!enabled) // if nav not enabled: stay at Load Page
    {
        m_contentStack->setCurrentIndex(0);
        m_sidebarList->setCurrentIndex(sidebarModel->index(0,0));
    } else // if nav enabled: jump to Overview?
    {
        m_contentStack->setCurrentIndex(1);
        m_sidebarList->setCurrentIndex(sidebarModel->index(1,0));
    }
}
void DbcFile::DbcView::onSidebarSelectionChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        return;
    }
    m_contentStack->setCurrentIndex(index.row());
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
    // Get application theme
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Create main layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Sidebar ---------------------------

    m_sidebarList = new QListView(this);
    m_sidebarList->setFixedWidth(200);
    m_sidebarList->setFrameShape(QFrame::NoFrame);
    m_sidebarList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sidebarList->setSelectionRectVisible(false);
    m_sidebarList->setStyleSheet(QString("QListView { "
                                         "background-color: %1;"
                                         " border-right: 1px solid %2; "
                                         " color: %3; "
                                         " font-size: %4px;"
                                         " outline: 0; "
                                         "}"

                                         "QListView::item {"
                                         "border-radius: %5px; "
                                         "padding: 20px; "
                                         "border: none;  "
                                         "} "

                                         "QListView::item:selected {"
                                         " background-color: %6;"
                                         " color: %7; "
                                         " border: none; "
                                         "}")
                                     .arg(colors.surfaceMain.name())
                                     .arg(colors.borderSubtle.name())
                                     .arg(colors.textSecondary.name())
                                     .arg(spacing.fontSizeMd)
                                     .arg(spacing.radiusSm)
                                     .arg(colors.surfacePrimary.name())
                                     .arg(colors.textPrimary.name()));

    // Create model for sidebar
    auto* sidebarModel = new QStandardItemModel(this);
    // 1. Load New item: always active, all other items initially inactive
    auto* itemLoad = new QStandardItem(QIcon(Constants::Sidebar::IconLoadNew), "Load New");
    sidebarModel->appendRow(itemLoad);

    // 2. Overview item
    auto* itemOverview = new QStandardItem(QIcon(Constants::Sidebar::IconOverview), "Overview");
    itemOverview->setEnabled(false);
    sidebarModel->appendRow(itemOverview);

    // 3. ECUs item
    auto* itemEcus = new QStandardItem(QIcon(Constants::Sidebar::IconEcus), "ECUs");
    itemEcus->setEnabled(false);
    sidebarModel->appendRow(itemEcus);

    // 4. Messages item
    auto* itemMessages = new QStandardItem(QIcon(Constants::Sidebar::IconMessages), "Messages");
    itemMessages->setEnabled(false);
    sidebarModel->appendRow(itemMessages);

    // 5. Signals item
    auto* itemSignals = new QStandardItem(QIcon(Constants::Sidebar::IconSignals), "Signals");
    itemSignals->setEnabled(false);
    sidebarModel->appendRow(itemSignals);

    m_sidebarList->setModel(sidebarModel);
    m_sidebarList->setItemDelegate(new SidebarDelegate(this));
    const QModelIndex firstIndex = sidebarModel->index(0, 0);
    m_sidebarList->setCurrentIndex(firstIndex);  // set "Load New" button selected at start
    mainLayout->addWidget(m_sidebarList);

    // 2. Content stack --------------------------

    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack);
    createSubViews();
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
    connect(m_sidebarList, &QListView::clicked, this, &DbcFile::DbcView::onSidebarSelectionChanged);
    connect(m_loadPage, &LoadPage::fileSelected, this, &DbcView::fileLoadRequested);
}
}  // namespace DbcFile