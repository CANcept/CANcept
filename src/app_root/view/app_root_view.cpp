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

#include "app_root/view/app_root_view.hpp"

#include <qevent.h>

#include <QEvent>
#include <QListView>
#include <QPainter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "app_root/constants.hpp"
#include "core/macro/console_logging.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

namespace AppRoot {

AppRootView::AppRootView(QWidget* parent)
    : QWidget(parent),
      m_tabView(new QListView(this)),
      m_contentStack(new QStackedWidget(this)),
      m_mainLayout(new QVBoxLayout(this)),
      m_topBarLayout(new QHBoxLayout()),
      m_logoLabel(new QLabel(this)),
      m_settingsButton(new QPushButton(this))
{
    resize(1200, 800);
    setupUi();
    applyStyle();
}

void AppRootView::setupUi()
{
    setObjectName("AppRootView");

    m_tabView->setFlow(QListView::LeftToRight);
    m_tabView->setViewMode(QListView::ListMode);
    m_tabView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_tabView->setWrapping(false);
    m_tabView->setUniformItemSizes(true);
    m_tabView->setFrameShape(QFrame::NoFrame);
    m_tabView->setFocusPolicy(Qt::StrongFocus);
    m_tabView->setAttribute(Qt::WA_StyledBackground);
    m_tabView->setMouseTracking(true);
    m_tabView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tabView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tabView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabView->viewport()->installEventFilter(this);
    m_tabView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setObjectName("settingsButton");
    connect(m_settingsButton, &QPushButton::clicked, this, &AppRootView::onSettingsClicked);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addLayout(m_topBarLayout);
    m_mainLayout->addWidget(m_contentStack);

    m_topBarLayout->addWidget(m_logoLabel);
    m_topBarLayout->addWidget(m_tabView, 1);
    m_topBarLayout->addWidget(m_settingsButton);

    constexpr QSize iconSize(22, 26);
    m_logoLabel->setFixedSize(iconSize);

    setLayout(m_mainLayout);
}

void AppRootView::applyStyle()
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    setStyleSheet(QString("#AppRootView {"
                          "  background-color: %1;"
                          "}")
                      .arg(colors.surfaceMain.name()));

    m_tabView->setFixedHeight(spacing.radiusMd * 2);
    m_tabView->setStyleSheet(QString("QListView {"
                                     "background-color: %1;"
                                     "border-radius: %2px;"
                                     "border: none;"
                                     "outline: none;"
                                     "}")
                                 .arg(colors.surfacePrimary.name())
                                 .arg(spacing.radiusMd));

    constexpr int settingsIconSize = 20;
    const QIcon settingsIcon(Constants::SETTINGS_ICON_PATH);
    QPixmap settingsPixmap =
        settingsIcon.pixmap(QSize(settingsIconSize, settingsIconSize), devicePixelRatioF());
    QPainter settingsPainter(&settingsPixmap);
    settingsPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    settingsPainter.fillRect(settingsPixmap.rect(), colors.textSecondary);
    settingsPainter.end();

    m_settingsButton->setIcon(QIcon(settingsPixmap));
    m_settingsButton->setIconSize(QSize(settingsIconSize, settingsIconSize));
    m_settingsButton->setFixedSize(spacing.radiusMd * 2, spacing.radiusMd * 2);
    updateSettingsButtonStyle(m_settingsActive);

    m_mainLayout->setSpacing(spacing.spacingXs);

    m_topBarLayout->setContentsMargins(spacing.spacingLg, spacing.spacingMd, spacing.spacingXl,
                                       spacing.spacingMd);
    m_topBarLayout->setSpacing(spacing.spacingXl);

    constexpr QSize iconSize(22, 26);
    const QIcon icon(Constants::CAN_BUS_ICON_PATH);
    QPixmap pixmap = icon.pixmap(iconSize, devicePixelRatioF());

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), colors.surfaceForeground);
    painter.end();

    m_logoLabel->setPixmap(pixmap);
}

void AppRootView::setModel(AppRootModel* model)
{
    m_model = model;
    m_tabView->setModel(m_model);

    if (m_model->rowCount() > 0)
    {
        onRowsInserted(QModelIndex(), 0, m_model->rowCount() - 1);
    }

    // Switch tabs on selection
    connect(m_tabView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                handleTabChanged(current.row());
            });

    // Disable clicking on no tab atall
    connect(m_tabView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
                m_tabView->viewport()->update();

                if (selected.isEmpty() && !deselected.isEmpty() && !m_settingsActive &&
                    m_model->rowCount() > 0)
                {
                    const QModelIndex lastIndex = deselected.first().indexes().first();
                    m_tabView->selectionModel()->select(lastIndex, QItemSelectionModel::Select);
                }
            });

    // Sync content: React to model updates
    connect(m_model, &AppRootModel::rowsInserted, this, &AppRootView::onRowsInserted);
    connect(m_model, &AppRootModel::rowsAboutToBeRemoved, this,
            &AppRootView::onRowsAboutToBeRemoved);
    connect(m_model, &AppRootModel::dataChanged, this, &AppRootView::onDataChanged);

    // Initialize the UI with the first tab selected
    if (m_model->rowCount() > 0)
    {
        const QModelIndex firstIndex = m_model->index(0, 0);
        m_tabView->selectionModel()->setCurrentIndex(
            firstIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void AppRootView::setDelegate(AppRootDelegate* delegate) const
{
    m_tabView->setItemDelegate(delegate);
}

void AppRootView::setSettingsModel(SettingsModel* settingsModel)
{
    m_settingsModel = settingsModel;
    m_settingsView = new SettingsView(m_settingsModel, this);
    m_contentStack->addWidget(m_settingsView);
}

void AppRootView::handleTabChanged(const int index)
{
    if (index < 0)
    {
        return;
    }

    LOG_INF("AppRoot", "Switching tab...")
    if (m_settingsActive)
    {
        m_settingsActive = false;
        updateSettingsButtonStyle(false);
    }

    if (index < m_contentStack->count() - 1)
    {
        m_lastTabIndex = index;
        m_contentStack->setCurrentIndex(index);
    }
}

void AppRootView::onSettingsClicked()
{
    if (m_settingsActive)
    {
        m_settingsActive = false;
        updateSettingsButtonStyle(false);
        m_tabView->selectionModel()->setCurrentIndex(
            m_model->index(m_lastTabIndex, 0),
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        m_contentStack->setCurrentIndex(m_lastTabIndex);
    } else
    {
        m_settingsActive = true;
        updateSettingsButtonStyle(true);
        if (m_tabView->selectionModel()->currentIndex().isValid())
        {
            m_lastTabIndex = m_tabView->selectionModel()->currentIndex().row();
        }
        m_tabView->selectionModel()->clear();
        if (m_settingsView)
        {
            m_settingsView->rebuild();
        }
        m_contentStack->setCurrentWidget(m_settingsView);
    }
}

void AppRootView::updateSettingsButtonStyle(bool active)
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    QString bgColor = active ? colors.surfaceSecondary.name() : "transparent";
    QString hoverColor = colors.surfaceSecondary.name();

    m_settingsButton->setStyleSheet(QString("QPushButton {"
                                            "  background-color: %1;"
                                            "  border: none;"
                                            "  border-radius: %2px;"
                                            "}"
                                            "QPushButton:hover {"
                                            "  background-color: %3;"
                                            "}")
                                        .arg(bgColor)
                                        .arg(spacing.radiusMd)
                                        .arg(hoverColor));
}

bool AppRootView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_tabView->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (const QModelIndex index = m_tabView->indexAt(mouseEvent->pos()); !index.isValid())
        {
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AppRootView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_model && m_tabView->selectionModel() &&
        !m_tabView->selectionModel()->currentIndex().isValid())
    {
        if (m_model->rowCount() > 0)
        {
            const QModelIndex firstIndex = m_model->index(0, 0);
            m_tabView->selectionModel()->setCurrentIndex(
                firstIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            handleTabChanged(0);
        }
    }
}

void AppRootView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_model)
    {
        emit m_model->layoutChanged();
    }
}

bool AppRootView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void AppRootView::onRowsInserted(const QModelIndex&, const int first, const int last) const
{
    for (int i = first; i <= last; ++i)
    {
        // Retrieve component from model to extract its widget
        if (auto* component = m_model->componentAt(i); component && component->getView())
        {
            m_contentStack->insertWidget(i, component->getView());
        }
    }
}

void AppRootView::onRowsAboutToBeRemoved(const QModelIndex&, int first, int last) const
{
    for (int i = last; i >= first; --i)
    {
        if (QWidget* widget = m_contentStack->widget(i))
        {
            m_contentStack->removeWidget(widget);
        }
    }
}
void AppRootView::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                const QList<int>& roles) const
{
    m_tabView->update();
}

}  // namespace AppRoot