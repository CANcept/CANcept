#include "messages_page.hpp"

#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#include "dbc_file/constants.hpp"
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
    const auto& spacing = THEME.spacing();
    // --- Main Layout ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                   spacing.spacingMd);
    mainLayout->setSpacing(spacing.spacingMd);

    // --- Header Card ---
    auto* card = new Core::CardWidget(Constants::SignalsPage::PageHeaderTitle,
                                      Constants::SignalsPage::PageHeaderSubtitle, "", this);
    mainLayout->addWidget(card);
    auto* cardLayout = card->layout();


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