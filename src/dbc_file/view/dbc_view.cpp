/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dbc_view.hpp"

#include <QVBoxLayout>

#include "core/widgets/sidebar.hpp"
#include "dbc_file/constants.hpp"

namespace DbcFile {

namespace {

// Creates, configures and rebuilds a flat proxy for a given item type.
auto makeFlatProxy(Core::DbcItemType type, QObject* parent, QAbstractItemModel* source)
    -> std::unique_ptr<FlatListProxy>
{
    auto proxy = std::make_unique<FlatListProxy>(type, parent);
    proxy->setSourceModel(source);
    proxy->rebuildMapping();
    return proxy;
}

}  // namespace

DbcView::DbcView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

DbcView::~DbcView() = default;

auto DbcView::getLoadPage() const -> LoadPage&
{
    return *m_loadPage;
}

void DbcView::setSignalUnits(const QStringList& units) const
{
    if (m_signalsPage) m_signalsPage->setAvailableUnits(units);
}

void DbcView::setAvailableSenders(const QStringList& senders) const
{
    if (m_messagesPage) m_messagesPage->setAvailableSenders(senders);
}

void DbcView::setSourceModel(QAbstractItemModel* model)
{
    if (!model) return;

    m_model = model;

    // Keep overview labels in sync whenever the model is rebuilt.
    connect(model, &QAbstractItemModel::modelReset, this,
            [this, model]() { m_overviewPage->updateLabels(model); });

    // --- Overview proxies (flat lists) ---
    m_ecuOverviewProxy = makeFlatProxy(Core::DbcItemType::Ecu, this, model);
    m_overviewPage->getEcuList()->setModel(m_ecuOverviewProxy.get());

    m_messageOverviewProxy = makeFlatProxy(Core::DbcItemType::Message, this, model);
    m_overviewPage->getMessageList()->setModel(m_messageOverviewProxy.get());

    // --- ECU page proxy (tree) ---
    m_ecuTreeProxy = std::make_unique<EcuTreeProxy>(this);
    m_ecuTreeProxy->setSourceModel(model);
    m_ecuPage->setModel(m_ecuTreeProxy.get());

    // --- Messages page proxies ---
    m_messagesProxy = makeFlatProxy(Core::DbcItemType::Message, this, model);
    m_messagesPage->setMasterModel(m_messagesProxy.get());
    m_messagesPage->setDetailModel(m_model);

    // --- Signals page proxy (flat list) ---
    m_signalsProxy = makeFlatProxy(Core::DbcItemType::Signal, this, model);
    m_signalsPage->setModel(m_signalsProxy.get());

    connect(model, &QAbstractItemModel::modelReset, this, [this]() {
        // A) Reset UI elements
        if (m_messagesPage) m_messagesPage->resetFilters();
        if (m_signalsPage) m_signalsPage->resetFilters();
        if (m_ecuPage) m_ecuPage->resetFilters();

        // B) Reset Proxy logic
        if (m_messagesProxy)
        {
            m_messagesProxy->setFilterMessageSender(QString());
            m_messagesProxy->setSearchFilter(QString());
        }
        if (m_signalsProxy)
        {
            m_signalsProxy->setSignalFilterUnit(QString());
            m_signalsProxy->setSearchFilter(QString());
        }
        if (m_ecuTreeProxy)
        {
            m_ecuTreeProxy->setSearchText(QString());
            m_ecuTreeProxy->setFilterCategory(0);
        }
    });
}

void DbcView::setNavigationEnabled(bool enabled) const
{
    m_sidebar->setNavigationEnabled(enabled);
}

void DbcView::onSidebarSelectionChanged(int index) const
{
    if (index < 0 || index >= m_contentStack->count()) return;
    if (m_contentStack->currentIndex() == index) return;

    // Reset load page UI state when leaving the load page.
    if (m_contentStack->currentIndex() == Constants::Sidebar::INDEX_LOAD &&
        index != Constants::Sidebar::INDEX_LOAD)
    {
        m_loadPage->resetStatus();
    }

    m_contentStack->setCurrentIndex(index);
}

void DbcView::onEcuFilterTextChanged(const QString& text) const
{
    if (m_ecuTreeProxy) m_ecuTreeProxy->setSearchText(text);
}

void DbcView::onEcuFilterIndexChanged(int index) const
{
    if (m_ecuTreeProxy) m_ecuTreeProxy->setFilterCategory(index);
}

void DbcView::onMessageFilterTextChanged(const QString& text)
{
    if (m_messagesProxy) m_messagesProxy->setSearchFilter(text);
}

void DbcView::onMessageSenderChanged(const QString& sender)
{
    if (m_messagesProxy) m_messagesProxy->setFilterMessageSender(sender);
}

void DbcView::onMessageSelected(const QModelIndex& proxyIndex)
{
    if (!m_messagesProxy) return;

    const QModelIndex sourceIndex = m_messagesProxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) return;

    // Ensure the detail view receives the row at column 0.
    m_messagesPage->selectMessageIndex(sourceIndex.siblingAtColumn(0));
}

void DbcView::onSignalFilterTextChanged(const QString& text)
{
    if (m_signalsProxy) m_signalsProxy->setSearchFilter(text);
}

void DbcView::onSignalUnitChanged(const QString& unit) const
{
    if (m_signalsProxy) m_signalsProxy->setSignalFilterUnit(unit);
}

void DbcView::addPage(QWidget* page, const QString& title, const QString& iconPath,
                      bool enabled) const
{
    m_contentStack->addWidget(page);
    m_sidebar->addTab(QIcon(iconPath), title, enabled);
}

void DbcView::setupSidebarTabs()
{
    // Pages are added in the same order as the sidebar tabs.
    m_loadPage = new LoadPage(this);
    addPage(m_loadPage, Constants::Sidebar::TitleLoadNew, Constants::Sidebar::IconLoadNew, true);

    m_overviewPage = new OverviewPage(this);
    addPage(m_overviewPage, Constants::Sidebar::TitleOverview, Constants::Sidebar::IconOverview,
            false);

    m_ecuPage = new EcusPage(this);
    addPage(m_ecuPage, Constants::Sidebar::TitleEcus, Constants::Sidebar::IconEcus, false);

    m_messagesPage = new MessagesPage(this);
    addPage(m_messagesPage, Constants::Sidebar::TitleMessages, Constants::Sidebar::IconMessages,
            false);

    m_signalsPage = new SignalsPage(this);
    addPage(m_signalsPage, Constants::Sidebar::TitleSignals, Constants::Sidebar::IconSignals,
            false);
}

void DbcView::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_sidebar = new Core::Sidebar(this);
    m_sidebar->setToolTipText(Constants::Sidebar::HoverText);
    mainLayout->addWidget(m_sidebar);

    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack);

    setupSidebarTabs();
    setupConnections();
}

void DbcView::setupConnections()
{
    connect(m_sidebar, &Core::Sidebar::tabSelected, this, &DbcView::onSidebarSelectionChanged);

    connect(m_loadPage, &LoadPage::fileSelected, this,
            [this](const QString& path) { emit fileLoadRequested(path); });

    connect(m_ecuPage, &EcusPage::filterTextChanged, this, &DbcView::onEcuFilterTextChanged);
    connect(m_ecuPage, &EcusPage::filterIndexChanged, this, &DbcView::onEcuFilterIndexChanged);

    connect(m_messagesPage, &MessagesPage::messageSelectionChanged, this,
            &DbcView::onMessageSelected);
    connect(m_messagesPage, &MessagesPage::masterFilterTextChanged, this,
            &DbcView::onMessageFilterTextChanged);
    connect(m_messagesPage, &MessagesPage::filterSenderChanged, this,
            &DbcView::onMessageSenderChanged);

    connect(m_signalsPage, &SignalsPage::filterTextChanged, this,
            &DbcView::onSignalFilterTextChanged);
    connect(m_signalsPage, &SignalsPage::filterUnitChanged, this, &DbcView::onSignalUnitChanged);
}

}  // namespace DbcFile
