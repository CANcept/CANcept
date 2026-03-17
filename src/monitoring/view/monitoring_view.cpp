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

#include "monitoring_view.hpp"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

#include "can_bus_config_card.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/tinted_icon_label.hpp"
#include "graph_list_view.hpp"
#include "monitoring/constants.hpp"
#include "monitoring/model/monitoring_model.hpp"

namespace Monitoring {

MonitoringView::MonitoringView(MonitoringModel* model, MonitoringDelegate* delegate,
                               QWidget* parent)
    : QWidget(parent),
      m_treeProxy(new QSortFilterProxyModel(this)),
      m_signalListView(new SignalList(this, model)),
      m_graphListView(new GraphListView(model, delegate)),
      m_splitter(new QSplitter(Qt::Horizontal, this)),
      m_model(model),
      m_delegate(delegate)
{
    setupUi();
}

void MonitoringView::setupUi()
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    mainLayout->setSpacing(spacing.spacingLg);

    // Configure Splitter
    m_splitter->addWidget(m_signalListView);
    m_splitter->addWidget(m_graphListView);

    // Give the Graph view more initial space (e.g., 30/70 split)
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    // --- Add to Main Layout ---
    mainLayout->addWidget(m_splitter);

    // Connect checkboxes in signalListview to graphlistview
    connect(m_signalListView, &SignalList::signalMonitoringToggled, m_graphListView,
            &GraphListView::signalChecked);

    // Create a fantastic awesome overlay
    m_noDbcOverlay = new QWidget(this);
    m_noDbcOverlay->setObjectName("noDbcOverlay");

    auto* overlayLayout = new QVBoxLayout(m_noDbcOverlay);
    overlayLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                      spacing.spacingLg);
    overlayLayout->addStretch(2);

    auto* messageContainer = new QWidget(m_noDbcOverlay);
    messageContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* messageLayout = new QHBoxLayout(messageContainer);
    messageLayout->setAlignment(Qt::AlignCenter);
    messageLayout->setContentsMargins(spacing.spacingMd, spacing.spacingMd, spacing.spacingMd,
                                      spacing.spacingMd);

    auto* messageLabel = new QLabel(Constants::DBC_FILE_NOT_LOADED_MESSAGE, messageContainer);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    messageLayout->addWidget(messageLabel, 1);

    m_settingsIconLabel = new Core::TintedIconLabel(Constants::SETTINGS_ICON_PATH, 24,
                                                    colors.textPrimary, messageContainer);
    messageLayout->addWidget(m_settingsIconLabel);

    auto* centeringLayout = new QHBoxLayout();
    centeringLayout->addStretch();
    centeringLayout->addWidget(messageContainer);
    centeringLayout->addStretch();
    overlayLayout->addLayout(centeringLayout);
    overlayLayout->addStretch(3);

    m_noDbcOverlay->show();
    m_noDbcOverlay->raise();

    applyStyle();
}

void MonitoringView::onUpdateMessages() const
{
    m_signalListView->updateViewData();
    m_graphListView->updateViewData();
}

void MonitoringView::applyStyle() const
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    if (m_noDbcOverlay)
    {
        m_noDbcOverlay->setStyleSheet(QString("QWidget#noDbcOverlay { "
                                              "background-color: rgba(%1, %2, %3, 140); "
                                              "}"
                                              "QLabel { "
                                              "color: %4; "
                                              "font-size: %5px; "
                                              "}")
                                          .arg(colors.surfaceMain.red())
                                          .arg(colors.surfaceMain.green())
                                          .arg(colors.surfaceMain.blue())
                                          .arg(colors.textPrimary.name())
                                          .arg(spacing.fontSizeLg));
    }

    if (m_settingsIconLabel)
    {
        m_settingsIconLabel->setColor(colors.textPrimary);
    }
}

void MonitoringView::showNoDbcOverlay() const
{
    if (m_noDbcOverlay)
    {
        m_noDbcOverlay->setGeometry(0, 0, width(), height());
        m_noDbcOverlay->show();
        m_noDbcOverlay->raise();
    }
}

void MonitoringView::hideNoDbcOverlay() const
{
    if (m_noDbcOverlay)
    {
        m_noDbcOverlay->hide();
    }
}

void MonitoringView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_noDbcOverlay)
    {
        m_noDbcOverlay->setGeometry(0, 0, width(), height());
    }
}

bool MonitoringView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Monitoring
