#include "sending_view.hpp"

#include <QHBoxLayout>
#include <QList>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "sending/constants.hpp"

namespace Sending {

SendingView::SendingView(QWidget* parent)
    : QWidget(parent),
      m_sidebarList(nullptr),
      m_contentStack(nullptr),
      m_rawView(nullptr),
      m_dbcView(nullptr)
{
    setupUi();
}

void SendingView::disableSidebarDeselection()
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

void SendingView::setupSidebarList()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_sidebarList = new QListView(this);
    m_sidebarList->setSelectionMode(QAbstractItemView::SingleSelection);
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

void SendingView::setSidebarModel()
{
    auto* sidebarModel = new QStandardItemModel(this);
    const QList<SidebarEntry> sidebarEntries = {
        {.iconPath = Constants::RAW_SENDING_ICON_PATH,
         .title = Constants::RAW_MODE_BUTTON_TEXT,
         .enabled = true},
        {.iconPath = Constants::DBC_SENDING_ICON_PATH,
         .title = Constants::DBC_MODE_BUTTON_TEXT,
         .enabled = true},
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
    m_sidebarList->setCurrentIndex(firstIndex);
}

void SendingView::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupSidebarList();
    setSidebarModel();
    mainLayout->addWidget(m_sidebarList);

    m_contentStack = new QStackedWidget(this);

    // Create sub-views
    m_rawView = new RawSendingSubView(m_contentStack);
    m_dbcView = new DbcSendingSubView(m_contentStack);

    m_contentStack->addWidget(m_rawView);  // Index 0
    m_contentStack->addWidget(m_dbcView);  // Index 1

    mainLayout->addWidget(m_contentStack, 1);

    // === Connect sidebar ===
    connect(m_sidebarList, &QListView::clicked, this, &SendingView::onSidebarSelectionChanged);
}

void SendingView::onSidebarSelectionChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    if (!(index.flags() & Qt::ItemIsEnabled))
    {
        return;
    }
    displayMode(index.row());
}

void SendingView::displayMode(int index)
{
    m_contentStack->setCurrentIndex(index);
    emit modeChanged(index == 1);
}

void SendingView::setModel(SendingModel* /*model*/)
{
    // Model binding can be implemented here if needed
    // For now, the component handles the connections directly
}

void SendingView::setAvailableDevices(const std::vector<std::string>& devices)
{
    if (m_rawView)
    {
        m_rawView->setAvailableInterfaces(devices);
    }
    if (m_dbcView)
    {
        m_dbcView->setAvailableInterfaces(devices);
    }
}

void SendingView::setAvailableSpeeds(const std::vector<uint32_t>& speeds)
{
    // Set baud rates for raw view
    if (m_rawView)
    {
        m_rawView->setAvailableBaudRates(speeds);
    }
    // DBC view doesn't have baud rate selection (uses global configuration)
}

}  // namespace Sending
