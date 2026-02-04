//
// Created by Adrian Rupp on 22.01.26.
//
#include "overview_page.hpp"

#include <QLineEdit>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/delegates/card_list_delegate.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/card_widget.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
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

    m_lblFileName->setText(getData(Constants::Columns::OvFilename));
    m_lblVersion->setText(getData(Constants::Columns::OvVersion));
    m_lblEcuCount->setText(getData(Constants::Columns::OvEcuCount));
    m_lblMessageCount->setText(getData(Constants::Columns::OvMsgCount));
    m_lblSignalCount->setText(getData(Constants::Columns::OvSigCount));
    m_lblOrphanCount->setText(getData(Constants::Columns::OvOrphans));
}
void OverviewPage::setupFileInfoSection(QVBoxLayout* parentLayout)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* fileInfoCard = new Core::CardWidget(Constants::OverviewPage::FileInfoTitle,
                                              Constants::OverviewPage::FileInfoSubTitle);
    auto fileInfoLayout = fileInfoCard->contentLayout();
    fileInfoLayout->setSpacing(spacing.spacingSm);
    fileInfoLayout->addSpacing(spacing.spacingLg);

    auto* grid = new QGridLayout();

    auto* fileNameTitle = new QLabel(Constants::OverviewPage::FileNameTitle);
    fileNameTitle->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_lblFileName = new QLabel(Constants::OverviewPage::LabelDefault);
    m_lblFileName->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    auto* fileVersionTitle = new QLabel(Constants::OverviewPage::FileVersionTitle);
    fileVersionTitle->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    m_lblVersion = new QLabel(Constants::OverviewPage::LabelDefault);
    m_lblVersion->setStyleSheet(QString("color: %1;").arg(colors.textSecondary.name()));

    grid->addWidget(fileNameTitle, 0, 0, Qt::AlignLeft);
    grid->addWidget(m_lblFileName, 0, 1, Qt::AlignRight);
    grid->addWidget(fileVersionTitle, 1, 0, Qt::AlignLeft);
    grid->addWidget(m_lblVersion, 1, 1, Qt::AlignRight);

    fileInfoLayout->addLayout(grid);
    parentLayout->addWidget(fileInfoCard);
}

void OverviewPage::setupStatsSection(QVBoxLayout* parentLayout)
{
    auto* layout = new QHBoxLayout();

    layout->addWidget(createStatCard(Constants::OverviewPage::EcuStatTitle, m_lblEcuCount,
                                     Constants::Sidebar::IconEcus));
    layout->addWidget(createStatCard(Constants::OverviewPage::MessagesStatTitle, m_lblMessageCount,
                                     Constants::Sidebar::IconMessages));
    layout->addWidget(createStatCard(Constants::OverviewPage::SignalsStatTitle, m_lblSignalCount,
                                     Constants::Sidebar::IconSignals));
    layout->addWidget(createStatCard(Constants::OverviewPage::OrphansStatTitle, m_lblOrphanCount,
                                     Constants::Sidebar::IconMessages));

    parentLayout->addLayout(layout);
}
void OverviewPage::createOverviewList(QHBoxLayout* parentLayout, const QString& title,
                                      QListView*& listViewMember, const QString& badgeIconPath)
{
    // Card container
    auto* listCard = new Core::CardWidget(title + Constants::OverviewPage::OverviewSuffix,
                                          Constants::OverviewPage::OverviewDescription.arg(title));

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
    constexpr int BadgeRole = DbcRoles::Role_ChildCount;
    constexpr int DetailRole = -1;  // no detail to item
    const auto badgeIcon = QIcon(badgeIconPath);

    auto* delegate = new Core::CardListDelegate(BadgeRole, badgeIcon,
                                                DetailRole,     // no column
                                                listViewMember  // parent → ownership
    );

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
                       Constants::Sidebar::IconMessages);

    createOverviewList(listsRowLayout, Constants::OverviewPage::MessagesStatTitle, m_messageList,
                       Constants::Sidebar::IconSignals);

    parentLayout->addLayout(listsRowLayout);
}

void OverviewPage::setupUi()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ScrollArea
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        QString("background-color: %1; width: 0px;").arg(colors.surfaceMain.name()));

    auto* contentWidget = new QWidget();
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(spacing.spacingSm);

    setupFileInfoSection(contentLayout);
    setupStatsSection(contentLayout);
    setupListsSection(contentLayout);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}
auto OverviewPage::createStatCard(const QString& title, QLabel*& valueLabelPtr,
                                  const QString& iconPath) -> QWidget*
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    // Card container
    auto* card = new Core::CardWidget();
    auto* cardLayout = card->contentLayout();
    card->setFixedHeight(spacing.HeightLg);

    // Top Row: Title + Icon
    auto* topRow = new QHBoxLayout();
    auto* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet(QString("color: %1; font-size: %2px;")
                                .arg(colors.textPrimary.name())
                                .arg(spacing.fontSizeMd));

    auto* iconLabel = new Core::TintedIconLabel(iconPath, spacing.IconMd,
                                                THEME.colors().textPrimary,  // Schwarz
                                                card);
    topRow->addWidget(lblTitle);
    topRow->addStretch();
    topRow->addWidget(iconLabel);
    cardLayout->addLayout(topRow);

    // Value
    valueLabelPtr = new QLabel(Constants::OverviewPage::LabelDefault);
    valueLabelPtr->setStyleSheet(QString("color: %1; font-weight: %2; font-size: %3px;")
                                     .arg(colors.textPrimary.name())
                                     .arg(spacing.fontWeightNormal)
                                     .arg(spacing.fontSizeLg));
    cardLayout->addWidget(valueLabelPtr);

    return card;
}

}  // namespace DbcFile
