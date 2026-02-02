//
// Created by Adrian Rupp on 22.01.26.
//
#include "overview_page.hpp"

#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

#include "core/delegates/card_list_delegate.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/theme_manager.hpp"
#include "core/widgets/card_widget.hpp"
#include "dbc_file/constants.hpp"
#include "dbc_file/model/dbc_roles.hpp"
#include "spdlog/fmt/bundled/os.h"
namespace DbcFile {
void OverviewPage::setFileName(const QString& text) const
{
    m_lblFileName->setText(text);
}
auto OverviewPage::setVersion(const QString& text) const -> void
{
    m_lblVersion->setText(text);
}
void OverviewPage::setEcuCount(const QString& text) const
{
    m_lblEcuCount->setText(text);
}
void OverviewPage::setMessageCount(const QString& text) const
{
    m_lblMessageCount->setText(text);
}
void OverviewPage::setSignalCount(const QString& text) const
{
    m_lblSignalCount->setText(text);
}
void OverviewPage::setOrphanCount(const QString& text) const
{
    m_lblOrphanCount->setText(text);
}
// --- OverviewPage Dummy ---
OverviewPage::OverviewPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
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

    auto* grid = new QGridLayout;

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

void OverviewPage::setupListsSection(QVBoxLayout* parentLayout)
{
    auto addList = [&](const QString& title, QListView*& listViewMember,
                       const QString& badgeIconPath) {
        auto* listCard =
            new Core::CardWidget(title + Constants::OverviewPage::OverviewSuffix,
                                 Constants::OverviewPage::OverviewDescription.arg(title));
        auto* layout = listCard->contentLayout();
        //
        listViewMember = new QListView(this);
        listViewMember->setViewMode(QListView::ListMode);
        listViewMember->setFlow(QListView::LeftToRight);
        listViewMember->setResizeMode(QListView::Adjust);
        listViewMember->setWrapping(true);
        listViewMember->setUniformItemSizes(true);
        listViewMember->setFrameShape(QFrame::NoFrame);
        listViewMember->setSelectionMode(QAbstractItemView::NoSelection);
        listViewMember->setStyleSheet(
            "QListView { "
            "  background: transparent; "
            "  border: none; "
            "  padding: 0px; "
            "  margin: 0px; "
            "}");
        const auto& spacing = THEME.spacing();
        listViewMember->setFixedHeight(spacing.HeightLg);

        // Set delegate
        int badgeRole = DbcRoles::Role_ChildCount;
        QIcon badgeIcon = QIcon(badgeIconPath);
        auto* delegate = new Core::CardListDelegate(badgeRole, badgeIcon, -1, listViewMember);
        listViewMember->setItemDelegate(delegate);

        layout->addWidget(listViewMember);
        parentLayout->addWidget(listCard);
    };
    addList(Constants::OverviewPage::EcuStatTitle, m_ecuList, Constants::Sidebar::IconMessages);

    addList(Constants::OverviewPage::MessagesStatTitle, m_messageList,
            Constants::Sidebar::IconSignals);
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

    setupFileInfoSection(contentLayout);
    setupStatsSection(contentLayout);
    setupListsSection(contentLayout);

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}
auto OverviewPage::createStatCard(const QString& title, QLabel*& valueLabelPtr,
                                  const QString& iconName) -> QWidget*
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
    lblTitle->setStyleSheet(QString("color: %1;").arg(colors.textPrimary.name()));

    auto* iconLabel = new QLabel();
    QIcon icon(iconName);
    if (!icon.isNull()) iconLabel->setPixmap(icon.pixmap(spacing.IconSm, spacing.IconSm));
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
