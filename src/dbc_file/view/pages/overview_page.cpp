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
    const auto& spacing = THEME.spacing();


    auto* listsRowLayout = new QHBoxLayout();
    listsRowLayout->setSpacing(spacing.spacingSm);

    auto addList = [&](const QString& title, QListView*& listViewMember,
                       const QString& badgeIconPath) {

        // Card Frame for the list
        auto* listCard =
            new Core::CardWidget(title + Constants::OverviewPage::OverviewSuffix,
                                 Constants::OverviewPage::OverviewDescription.arg(title));
        listCard->setSizePolicy(
                            QSizePolicy::Expanding,
                            QSizePolicy::Expanding
                                );
        auto* layout = listCard->contentLayout();

        // Initalize listView
        listViewMember = new QListView(listCard);

        listViewMember->setViewMode(QListView::ListMode);
        listViewMember->setFlow(QListView::TopToBottom);
        listViewMember->setWrapping(false);
        listViewMember->setFrameShape(QFrame::NoFrame);
        listViewMember->setSelectionMode(QAbstractItemView::NoSelection);
        listViewMember->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        listViewMember->setSizePolicy(
            QSizePolicy::Expanding,
            QSizePolicy::Expanding
        );

        // Style
        listViewMember->setStyleSheet("background: transparent; border: none; padding: 0px; margin: 0px;");


        // Set delegate
        const int badgeRole = DbcRoles::Role_ChildCount;
        const auto badgeIcon = QIcon(badgeIconPath);
        auto* delegate = new Core::CardListDelegate(badgeRole, badgeIcon, -1, listViewMember);
        listViewMember->setItemDelegate(delegate);

        // Add List to layout
        layout->addWidget(listViewMember);
        listsRowLayout->addWidget(listCard);
    };
    // Add to lists two parent layout
    addList(Constants::OverviewPage::EcuStatTitle, m_ecuList, Constants::Sidebar::IconMessages);
    addList(Constants::OverviewPage::MessagesStatTitle, m_messageList,
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
    lblTitle->setStyleSheet(QString("color: %1; font-size: %2px;").arg(colors.textPrimary.name()).arg(spacing.fontSizeMd));


    auto* iconLabel = new QLabel();
    if (const QIcon icon(iconName); !icon.isNull()) {
        QPixmap pix = icon.pixmap(24, 24);

        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), THEME.colors().textPrimary);
        p.end();

        iconLabel->setPixmap(pix);
    }
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
