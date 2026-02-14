#include "overview_page.hpp"

#include <QLineEdit>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

#include "core/delegates/card_list_delegate.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "dbc_file/styles.hpp"
#include "spdlog/fmt/bundled/os.h"
namespace DbcFile {
OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void OverviewPage::updateLabels(const QAbstractItemModel* model) const
{
    if (!model || model->rowCount() == 0) return;
    if (const QModelIndex overviewIndex = model->index(0, 0, QModelIndex());
        !overviewIndex.isValid())
        return;

    auto getData = [&](const int col) {
        return model->data(model->index(0, col), Qt::DisplayRole).toString();
    };

    if (!getData(Constants::Columns::OvFilename).isEmpty())
    {
        m_lblFileName->setText(getData(Constants::Columns::OvFilename));
    }
    if (!getData(Constants::Columns::OvVersion).isEmpty())
    {
        m_lblVersion->setText(getData(Constants::Columns::OvVersion));
    }
    m_lblEcuCount->setText(getData(Constants::Columns::OvEcuCount));
    m_lblMessageCount->setText(getData(Constants::Columns::OvMsgCount));
    m_lblSignalCount->setText(getData(Constants::Columns::OvSigCount));
    m_lblOrphanCount->setText(getData(Constants::Columns::OvOrphans));
}
void OverviewPage::setupFileInfoSection(QVBoxLayout* parentLayout)
{
    const auto& spacing = THEME.spacing();

    auto* fileInfoCard = new Core::CardWidget(Constants::OverviewPage::FileInfoTitle,
                                              Constants::OverviewPage::FileInfoSubTitle, "", this);
    auto fileInfoLayout = fileInfoCard->contentLayout();
    fileInfoLayout->setSpacing(spacing.spacingSm);
    fileInfoLayout->addSpacing(spacing.spacingLg);

    auto* grid = new QGridLayout();

    m_fileNameTitle = new QLabel(Constants::OverviewPage::FileNameTitle, fileInfoCard);
    m_lblFileName = new QLabel(Constants::OverviewPage::LabelDefault, fileInfoCard);
    m_fileVersionTitle = new QLabel(Constants::OverviewPage::FileVersionTitle, fileInfoCard);
    m_lblVersion = new QLabel(Constants::OverviewPage::LabelDefault, fileInfoCard);

    grid->addWidget(m_fileNameTitle, 0, 0, Qt::AlignLeft);
    grid->addWidget(m_lblFileName, 0, 1, Qt::AlignRight);
    grid->addWidget(m_fileVersionTitle, 1, 0, Qt::AlignLeft);
    grid->addWidget(m_lblVersion, 1, 1, Qt::AlignRight);

    fileInfoLayout->addLayout(grid);
    parentLayout->addWidget(fileInfoCard);
}

void OverviewPage::setupStatsSection(QVBoxLayout* parentLayout)
{
    auto* layout = new QHBoxLayout();

    layout->addWidget(createStatCard(Constants::OverviewPage::EcuStatTitle, m_lblEcuCount,
                                     Constants::Sidebar::IconEcus, m_statTitleLabels,
                                     m_statIconLabels, this));
    layout->addWidget(createStatCard(Constants::OverviewPage::MessagesStatTitle, m_lblMessageCount,
                                     Constants::Sidebar::IconMessages, m_statTitleLabels,
                                     m_statIconLabels, this));
    layout->addWidget(createStatCard(Constants::OverviewPage::SignalsStatTitle, m_lblSignalCount,
                                     Constants::Sidebar::IconSignals, m_statTitleLabels,
                                     m_statIconLabels, this));
    layout->addWidget(createStatCard(Constants::OverviewPage::OrphansStatTitle, m_lblOrphanCount,
                                     Constants::Sidebar::IconMessages, m_statTitleLabels,
                                     m_statIconLabels, this));

    parentLayout->addLayout(layout);
}
void OverviewPage::createOverviewList(QHBoxLayout* parentLayout, const QString& title,
                                      QListView*& listViewMember, const QString& badgeIconPath,
                                      QWidget* parent)
{
    // Card container
    auto* listCard =
        new Core::CardWidget(title + Constants::OverviewPage::OverviewSuffix,
                             Constants::OverviewPage::OverviewDescription.arg(title), "", parent);

    listCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = listCard->contentLayout();

    // Create and configure list view
    listViewMember = new QListView(listCard);
    listViewMember->setViewMode(QListView::ListMode);
    listViewMember->setFlow(QListView::TopToBottom);
    listViewMember->setWrapping(false);
    listViewMember->setFrameShape(QFrame::NoFrame);
    listViewMember->setSelectionMode(QAbstractItemView::NoSelection);
    listViewMember->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    listViewMember->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    listViewMember->setStyleSheet(
        "background: transparent; border: none; padding: 0px; margin: 0px;");

    // Delegate
    constexpr int BadgeRole = Role_ChildCount;
    constexpr int DetailRole = -1;  // no detail to item
    const auto badgeIcon = QIcon(badgeIconPath);

    auto* delegate = new Core::CardListDelegate(BadgeRole, badgeIcon,
                                                DetailRole,  // no column
                                                listViewMember);

    listViewMember->setItemDelegate(delegate);

    // Assemble
    layout->addWidget(listViewMember);
    parentLayout->addWidget(listCard);
}

void OverviewPage::setupListsSection(QVBoxLayout* parentLayout)
{
    const auto& spacing = THEME.spacing();

    auto* listsRowLayout = new QHBoxLayout();
    listsRowLayout->setSpacing(spacing.spacingSm);

    createOverviewList(listsRowLayout, Constants::OverviewPage::EcuStatTitle, m_ecuList,
                       Constants::Sidebar::IconMessages, this);

    createOverviewList(listsRowLayout, Constants::OverviewPage::MessagesStatTitle, m_messageList,
                       Constants::Sidebar::IconSignals, this);

    parentLayout->addLayout(listsRowLayout);
}

void OverviewPage::setupUi()
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd, spacing.spacingMd);

    // ScrollArea
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(spacing.spacingSm);

    setupFileInfoSection(contentLayout);
    setupStatsSection(contentLayout);
    setupListsSection(contentLayout);

    m_scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(m_scrollArea);

    applyStyle();
}
auto OverviewPage::createStatCard(const QString& title, QLabel*& valueLabelPtr,
                                  const QString& iconPath, std::vector<QLabel*>& titleLabelsOut,
                                  std::vector<Core::TintedIconLabel*>& iconLabelsOut,
                                  QWidget* parent) -> QWidget*
{
    const auto& spacing = THEME.spacing();

    // Card container
    auto* card = new Core::CardWidget("", "", "", parent);
    auto* cardLayout = card->contentLayout();
    card->setFixedHeight(spacing.HeightLg);

    // Top Row: Title + Icon
    auto* topRow = new QHBoxLayout();
    auto* lblTitle = new QLabel(title, card);
    titleLabelsOut.push_back(lblTitle);

    auto* iconLabel = new Core::TintedIconLabel(iconPath, spacing.IconMd, Qt::black, card);
    iconLabelsOut.push_back(iconLabel);

    topRow->addWidget(lblTitle);
    topRow->addStretch();
    topRow->addWidget(iconLabel);
    cardLayout->addLayout(topRow);

    // Value
    valueLabelPtr = new QLabel(Constants::OverviewPage::LabelDefault, card);
    titleLabelsOut.push_back(valueLabelPtr);
    cardLayout->addWidget(valueLabelPtr);

    return card;
}
void OverviewPage::applyStyle() const
{
    if (m_scrollArea)
        m_scrollArea->setStyleSheet(Style::OverviewPage::scrollArea());

    if (m_fileNameTitle)
        m_fileNameTitle->setStyleSheet(Style::OverviewPage::secondaryLabel());

    if (m_lblFileName)
        m_lblFileName->setStyleSheet(Style::OverviewPage::secondaryLabel());

    if (m_fileVersionTitle)
        m_fileVersionTitle->setStyleSheet(Style::OverviewPage::secondaryLabel());

    if (m_lblVersion)
        m_lblVersion->setStyleSheet(Style::OverviewPage::secondaryLabel());

    for (size_t i = 0; i < m_statTitleLabels.size(); ++i)
    {
        if (i % 2 == 0)
            m_statTitleLabels[i]->setStyleSheet(
                Style::OverviewPage::statTitle());
        else
            m_statTitleLabels[i]->setStyleSheet(
                Style::OverviewPage::statValue());
    }

    for (auto* iconLabel : m_statIconLabels)
        if (iconLabel)
            iconLabel->setColor(THEME.colors().textPrimary);
}
// void OverviewPage::applyStyle() const
// {
//     const auto& colors = THEME.colors();
//     const auto& spacing = THEME.spacing();
//
//     if (m_scrollArea)
//     {
//         m_scrollArea->setStyleSheet(
//             QString("background-color: %1; width: 0px;").arg(colors.surfaceMain.name()));
//     }
//
//     if (m_fileNameTitle)
//     {
//         m_fileNameTitle->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
//     }
//     if (m_lblFileName)
//     {
//         m_lblFileName->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
//     }
//     if (m_fileVersionTitle)
//     {
//         m_fileVersionTitle->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
//     }
//     if (m_lblVersion)
//     {
//         m_lblVersion->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));
//     }
//
//     for (size_t i = 0; i < m_statTitleLabels.size(); ++i)
//     {
//         if (i % 2 == 0)
//         {
//             m_statTitleLabels[i]->setStyleSheet(QString("color: %1; font-size: %2px;")
//                                                     .arg(colors.textPrimary.name())
//                                                     .arg(spacing.fontSizeMd));
//         } else
//         {
//             m_statTitleLabels[i]->setStyleSheet(
//                 QString("color: %1; font-weight: %2; font-size: %3px;")
//                     .arg(colors.textPrimary.name())
//                     .arg(spacing.fontWeightNormal)
//                     .arg(spacing.fontSizeLg));
//         }
//     }
//
//     for (auto* iconLabel : m_statIconLabels)
//     {
//         if (iconLabel)
//         {
//             iconLabel->setColor(colors.textPrimary);
//         }
//     }
// }

bool OverviewPage::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace DbcFile
