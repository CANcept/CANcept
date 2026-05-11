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

#include "logging_detail_view.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QVBoxLayout>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "logging/constants.hpp"
#include "logging/styles.hpp"

namespace Logging {

LoggingDetailView::LoggingDetailView(LoggingModel* model, const QModelIndex& sessionIndex,
                                     QWidget* parent)
    : QWidget(parent)
{
    m_proxy = new LoggingDetailProxy(this);
    m_proxy->setSourceModel(model);
    m_proxy->setSessionIndex(sessionIndex);
    const LogSession* session = model->getSession(model->sessionIdAt(sessionIndex));
    setupUi(session);
    applyStyle();
}

void LoggingDetailView::setupUi(const LogSession* session)
{
    const auto& spacing = THEME.spacing();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(spacing.spacingMd);

    m_infoCard = new Core::CardWidget(Constants::DETAIL_INFO_CARD_TITLE, {},
                                      Constants::INFO_ICON_PATH, this);

    auto* infoContainer = new QWidget(m_infoCard);
    m_infoGrid = new QGridLayout(infoContainer);
    m_infoGrid->setContentsMargins(0, 0, 0, 0);
    m_infoGrid->setSpacing(spacing.spacingLg);
    m_infoGrid->setColumnStretch(0, 1);
    m_infoGrid->setColumnStretch(1, 1);

    const auto makeCell = [&](const QString& key, const QString& value) -> QWidget* {
        auto* cell = new QWidget(infoContainer);
        auto* cellLayout = new QVBoxLayout(cell);
        cellLayout->setContentsMargins(0, 0, 0, 0);
        cellLayout->setSpacing(spacing.spacingXs);
        auto* keyLabel = new QLabel(key, cell);
        keyLabel->setObjectName("infoKey");
        auto* valLabel = new QLabel(value, cell);
        valLabel->setObjectName("infoValue");
        cellLayout->addWidget(keyLabel);
        cellLayout->addWidget(valLabel);
        return cell;
    };

    m_infoGrid->addWidget(makeCell(Constants::DETAIL_LABEL_SESSION_ID, session->id), 0, 0);
    m_infoGrid->addWidget(
        makeCell(Constants::DETAIL_LABEL_TIMESTAMP,
                 session->startDateTime.toString(Constants::DETAIL_DATETIME_FORMAT)),
        0, 1);
    m_infoGrid->addWidget(makeCell(Constants::DETAIL_LABEL_DURATION, session->duration), 1, 0);
    m_infoGrid->addWidget(
        makeCell(Constants::DETAIL_LABEL_MESSAGES, QString::number(m_proxy->totalRecordCount())), 1,
        1);

    m_infoCard->contentLayout()->addWidget(infoContainer);
    mainLayout->addWidget(m_infoCard);

    m_contentCard = new Core::CardWidget(Constants::DETAIL_DATA_CARD_TITLE, {},
                                         Constants::DATA_ICON_PATH, this);

    m_tableView = new QTableView(m_contentCard);
    m_tableView->setModel(m_proxy);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(false);
    m_tableView->setShowGrid(false);
    m_tableView->setFrameShape(QFrame::NoFrame);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->verticalHeader()->hide();
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_contentCard->contentLayout()->addWidget(m_tableView, 1);

    auto* paginatorBar = new QWidget(m_contentCard);
    auto* paginatorLayout = new QHBoxLayout(paginatorBar);
    paginatorLayout->setContentsMargins(0, 0, 0, 0);
    paginatorLayout->setSpacing(spacing.spacingMd);

    m_prevBtn = new QPushButton(Constants::DETAIL_PREV_BTN_LABEL, paginatorBar);
    m_prevBtn->setMouseTracking(true);
    m_pageLabel = new QLabel(paginatorBar);
    m_pageLabel->setAlignment(Qt::AlignCenter);
    m_nextBtn = new QPushButton(Constants::DETAIL_NEXT_BTN_LABEL, paginatorBar);
    m_nextBtn->setMouseTracking(true);

    paginatorLayout->addStretch();
    paginatorLayout->addWidget(m_prevBtn);
    paginatorLayout->addWidget(m_pageLabel);
    paginatorLayout->addWidget(m_nextBtn);
    paginatorLayout->addStretch();

    m_contentCard->contentLayout()->addWidget(paginatorBar);
    mainLayout->addWidget(m_contentCard, 1);

    // Back Button
    auto* backBar = new QWidget(this);
    auto* backBarLayout = new QHBoxLayout(backBar);
    backBarLayout->setContentsMargins(0, 0, 0, 0);
    m_backBtn = new QPushButton(Constants::DETAIL_BACK_BTN_LABEL, backBar);
    backBarLayout->addWidget(m_backBtn);
    backBarLayout->addStretch();
    mainLayout->addWidget(backBar);

    updatePaginator();

    connect(m_backBtn, &QPushButton::clicked, this, &LoggingDetailView::backRequested);
    connect(m_prevBtn, &QPushButton::clicked, this, [this]() -> void {
        if (!m_proxy->hasPrevPage()) return;
        const auto offset = static_cast<uint64_t>(m_proxy->currentPage() - 2) *
                            static_cast<uint64_t>(Constants::DETAIL_PAGE_SIZE);
        m_proxy->loadPage(offset);
        updatePaginator();
    });
    connect(m_nextBtn, &QPushButton::clicked, this, [this]() -> void {
        if (!m_proxy->hasNextPage()) return;
        const auto offset = static_cast<uint64_t>(m_proxy->currentPage()) *
                            static_cast<uint64_t>(Constants::DETAIL_PAGE_SIZE);
        m_proxy->loadPage(offset);
        updatePaginator();
    });
}

void LoggingDetailView::updatePaginator() const
{
    m_prevBtn->setEnabled(m_proxy->hasPrevPage());
    m_nextBtn->setEnabled(m_proxy->hasNextPage());
    m_pageLabel->setText(QString("%1 / %2").arg(m_proxy->currentPage()).arg(m_proxy->pageCount()));
}

void LoggingDetailView::applyStyle() const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_infoGrid->parentWidget()->setStyleSheet(
        QString("QLabel#infoKey {"
                "  color: %1; font-size: %2px; border: none; background: transparent;"
                "}"
                "QLabel#infoValue {"
                "  color: %3; font-size: %4px; font-weight: %5;"
                "  border: none; background: transparent;"
                "}")
            .arg(colors.textSecondary.name())
            .arg(spacing.fontSizeSm)
            .arg(colors.textPrimary.name())
            .arg(spacing.fontSizeMd)
            .arg(spacing.fontWeightMedium));

    m_tableView->setStyleSheet(QString(R"(
            QTableView {
                border: none;
                background: transparent;
                color: %1;
                selection-background-color: %2;
                selection-color: %1;
                outline: none;
            }
            QTableView::viewport {
                background: transparent;
            }
            QTableView::item {
                background: transparent;
                padding: %3px;
                border: none;
            }
            QTableView::item:selected {
                background-color: %2;
                color: %1;
            }
            QHeaderView {
                background: transparent;
            }
            QHeaderView::section {
                background: transparent;
                color: %1;
                border: none;
                border-bottom: %4px solid %5;
                padding: %3px;
                font-weight: bold;
                font-size: %6px;
            }
        )")
                                   .arg(colors.textPrimary.name())
                                   .arg(colors.surfaceSelected.name())
                                   .arg(spacing.spacingXs)
                                   .arg(spacing.borderThick)
                                   .arg(colors.borderSubtle.name(QColor::HexArgb))
                                   .arg(spacing.fontSizeMd));

    m_tableView->verticalScrollBar()->setStyleSheet(Style::Common::verticalScrollBar());
    m_tableView->horizontalScrollBar()->setStyleSheet(Style::Common::horizontalScrollBar());

    const QString btnStyle = QString(
                                 "QPushButton {"
                                 "  background-color: %1; "
                                 "  border-radius: %3px;"
                                 "  color: %2; font-size: %4px; padding: %5px %6px;"
                                 "}"
                                 "QPushButton:hover { background-color: %7; }"
                                 "QPushButton:disabled { color: %8; }")
                                 .arg(colors.surfacePrimary.name())
                                 .arg(colors.textPrimary.name())
                                 .arg(spacing.radiusXs)
                                 .arg(spacing.fontSizeMd)
                                 .arg(spacing.spacingXs)
                                 .arg(spacing.spacingMd)
                                 .arg(colors.surfaceHover.name())
                                 .arg(colors.textDisabled.name());

    m_prevBtn->setStyleSheet(btnStyle);
    m_nextBtn->setStyleSheet(btnStyle);

    m_pageLabel->setStyleSheet(QString("QLabel { color: %1; font-size: %2px; }")
                                   .arg(colors.textSecondary.name())
                                   .arg(spacing.fontSizeMd));

    m_backBtn->setStyleSheet(QString("QPushButton {"
                                     "   background-color: %1;"
                                     "   border: none;"
                                     "   border-radius: %2px;"
                                     "   font-size: %3px;"
                                     "   font-weight: %4;"
                                     "   padding: %5px %6px;"
                                     "}"
                                     "QPushButton:hover { background-color: %7; }")
                                 .arg(colors.colorPrimary.name())
                                 .arg(spacing.radiusSm)
                                 .arg(spacing.fontSizeMd)
                                 .arg(spacing.fontWeightMedium)
                                 .arg(spacing.spacingMd)
                                 .arg(spacing.spacingLg * 2)
                                 .arg(colors.colorPrimaryHover.name()));
}

auto LoggingDetailView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Logging