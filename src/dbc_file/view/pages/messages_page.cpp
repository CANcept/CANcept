#include "messages_page.hpp"
namespace DbcFile {
// --- MessagesPage Dummy ---
MessagesPage::MessagesPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
void MessagesPage::setMasterModel(QAbstractItemModel* model) {}
void MessagesPage::setDetailModel(QAbstractItemModel* model) {}
void MessagesPage::setSignalDelegate(QAbstractItemDelegate* delegate) {}
void MessagesPage::showDetailsPane(bool visible) {}
auto MessagesPage::getMasterFilterCombo() const -> QComboBox*
{
    return nullptr;
}
void MessagesPage::setDetailTitle(const QString& title) {}
void MessagesPage::onSelectionChanged(const QModelIndex& current, const QModelIndex& previous) {}
void MessagesPage::setupUi()
{
    m_splitter = new QSplitter(this);
    m_messagesTable = new Core::SearchableFilterTable(this);
    m_detailView = new MessageDetailView(this);
}

// --- MessageDetailView Dummy ---
MessageDetailView::MessageDetailView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}
auto MessageDetailView::getSignalList() const -> QListView*
{
    return m_signalList;
}
void MessageDetailView::setMessageTitle(const QString& title) {}
void MessageDetailView::setupUi()
{
    m_signalList = new QListView(this);
    m_lblTitle = new QLabel(this);
}
}  // namespace DbcFile