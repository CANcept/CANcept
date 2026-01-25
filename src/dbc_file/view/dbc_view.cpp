//
// Created by Adrian Rupp on 20.01.26.
#include "dbc_view.hpp"

#include <QList>
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
void DbcFile::DbcView::setNavigationEnabled(const bool enabled) const
{
    // Get Model of m_sideBar (QListView)
    const auto* sidebarModel =
        static_cast<QStandardItemModel*>(m_sidebarList->model());

    // Enable (or disable) all items (~pages)
    for (int i = 1; i < sidebarModel->rowCount(); i++)
    {
        if (auto* item = sidebarModel->item(i))
        {
            item->setEnabled(enabled);
            item->setSelectable(enabled);
        }
    }
}
void DbcFile::DbcView::onSidebarSelectionChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    if (!(index.flags() & Qt::ItemIsEnabled)) // do nothing if item is already enabled
    {
        return;
    }
    if (m_contentStack->currentIndex() == 0 && index.row() != 0)
    {
        m_loadPage->resetStatus();
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

void DbcView::disableSidebarDeselection()
{
    // Get selection model of m_sidebarList
    auto* selectionModel = m_sidebarList->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            [selectionModel](const QItemSelection& selected, const QItemSelection& deselected) {
                // Check: is new selection empty? (~ click in empty space)
                if (selected.indexes().isEmpty())
                {
                    // Reselect previous selection again
                    if (!deselected.indexes().isEmpty())
                    {
                        selectionModel->select(
                            deselected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                        selectionModel->setCurrentIndex(deselected.indexes().first(),
                                                        QItemSelectionModel::NoUpdate);
                    }
                }
            });
}

void DbcView::setupSidebarList()
{
    const auto& THEME = Core::ThemeManager::getInstance();
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_sidebarList = new QListView(this);
    m_sidebarList->setSelectionMode(
    QAbstractItemView::SingleSelection);
    m_sidebarList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sidebarList->setMaximumWidth(200);
    m_sidebarList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_sidebarList->setFrameShape(QFrame::NoFrame);
    m_sidebarList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sidebarList->setSelectionRectVisible(false);
    m_sidebarList->setStyleSheet(QString(R"(
                                                QListView {
                                                    background-color: %1;
                                                    border-right: %2px solid %3;
                                                    color: %4;
                                                    font-size: %5px;
                                                    outline: 0;
                                                }

                                                QListView::item {
                                                    border-radius: %6px;
                                                    padding: %7px;
                                                    margin-right: %8px;
                                                    margin-left: %8px;
                                                }

                                                QListView::item:selected {
                                                    background-color: %9;
                                                    color: %10;
                                                }
                                            )")
                                     .arg(colors.surfaceMain.name(QColor::HexArgb))
                                     .arg(spacing.borderThick)
                                     .arg(colors.borderSubtle.name(QColor::HexArgb))
                                     .arg(colors.textSecondary.name(QColor::HexArgb))
                                     .arg(spacing.fontSizeMd)
                                     .arg(spacing.radiusSm)
                                     .arg(spacing.spacingXl)
                                     .arg(spacing.spacingMd)
                                     .arg(colors.surfacePrimary.name(QColor::HexArgb))
                                     .arg(colors.textPrimary.name(QColor::HexArgb)));
}

void DbcView::setSidebarModel()
{
    auto* sidebarModel = new QStandardItemModel(this);
    const QList<SidebarEntry> sidebarEntries = {
        { .iconPath=Constants::Sidebar::IconLoadNew, .title=Constants::Sidebar::TitleLoadNew, .enabled=true  },
        { .iconPath=Constants::Sidebar::IconOverview, .title=Constants::Sidebar::TitleOverview, .enabled=false },
        { .iconPath=Constants::Sidebar::IconEcus,     .title=Constants::Sidebar::TitleEcus,     .enabled=false },
        { .iconPath=Constants::Sidebar::IconMessages, .title=Constants::Sidebar::TitleMessages, .enabled=false },
        { .iconPath=Constants::Sidebar::IconSignals,  .title=Constants::Sidebar::TitleSignals,  .enabled=false },
    };

    for (const auto& entry : sidebarEntries)
    {
        auto* item = new QStandardItem(QIcon(entry.iconPath), entry.title);
        item->setEnabled(entry.enabled);
        item->setSelectable(entry.enabled);
        sidebarModel->appendRow(item);
    }

    m_sidebarList->setModel(sidebarModel);
    disableSidebarDeselection();
    const QModelIndex firstIndex = sidebarModel->index(0, 0);
    m_sidebarList->setCurrentIndex(firstIndex);  // set "Load New" button selected at start
}
void DbcFile::DbcView::setupUi()
{
    // Create main layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Sidebar setup
    setupSidebarList();
    setSidebarModel();
    m_sidebarList->setItemDelegate(new SidebarDelegate(m_sidebarList));
    mainLayout->addWidget(m_sidebarList);

    // Content setup
    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack);
    createSubViews();

    // Signal connections
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
    connect(m_loadPage, &LoadPage::fileSelected, this, [this] (const QString& path) {
        emit fileLoadRequested(path);
    });
}
}  // namespace DbcFile