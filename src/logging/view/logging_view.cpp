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

#include "logging_view.hpp"

#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QSpacerItem>

#include "components/logging_detail_view.hpp"
#include "components/message_selection_dialog.hpp"
#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"
#include "core/widgets/tinted_icon_label.hpp"

namespace Logging {

LoggingView::LoggingView(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void LoggingView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 16, 40, 16);
    mainLayout->setSpacing(12);

    // ===== Header Section =====
    m_headerBox = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_headerBox);
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerLayout->setSpacing(10);

    // Action Button (Start/Stop)
    m_btnAction = new StartStopButton(m_headerBox);
    headerLayout->addWidget(m_btnAction);

    // Timer Label
    m_timerLabel = new TimerLabel(m_headerBox);
    headerLayout->addWidget(m_timerLabel);

    headerLayout->addStretch();

    mainLayout->addWidget(m_headerBox);

    // ===== Main Frame (bordered container) =====
    m_mainFrame = new QFrame(this);
    m_mainFrame->setFrameShape(QFrame::StyledPanel);
    m_mainFrame->setObjectName("loggingMainFrame");

    auto* frameLayout = new QVBoxLayout(m_mainFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    // ===== Content Stack (History Table <-> Detail View) =====
    m_contentStack = new QStackedWidget(m_mainFrame);

    // --- History Page ---
    m_historyPage = new QWidget(m_contentStack);
    auto* historyLayout = new QVBoxLayout(m_historyPage);
    historyLayout->setContentsMargins(10, 10, 10, 10);

    m_historyTable = new LogHistoryTable(m_historyPage);
    historyLayout->addWidget(m_historyTable);

    // Empty state label
    m_emptyLabel = new NoLogsLabel(m_historyPage);
    historyLayout->addWidget(m_emptyLabel);

    m_contentStack->addWidget(m_historyPage);

    // --- Detail Page ---
    m_detailPage = new QWidget(m_contentStack);
    m_detailLayout = new QVBoxLayout(m_detailPage);
    m_detailLayout->setContentsMargins(10, 10, 10, 10);
    m_detailLayout->setSpacing(10);

    m_contentStack->addWidget(m_detailPage);

    // Show history by default
    m_contentStack->setCurrentWidget(m_historyPage);

    frameLayout->addWidget(m_contentStack);
    mainLayout->addWidget(m_mainFrame, 1);

    // ===== Device Not Configured Overlay =====
    m_deviceNotConfiguredOverlay = new QWidget(this);
    m_deviceNotConfiguredOverlay->setObjectName("deviceNotConfiguredOverlay");

    auto* overlayLayout = new QVBoxLayout(m_deviceNotConfiguredOverlay);

    overlayLayout->addStretch(2);

    auto* messageContainer = new QWidget(m_deviceNotConfiguredOverlay);
    auto* messageLayout = new QHBoxLayout(messageContainer);
    messageLayout->setAlignment(Qt::AlignCenter);

    auto* messageLabel = new QLabel("Connection first has to be configured in ", messageContainer);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLayout->addWidget(messageLabel);

    const auto& colors = THEME.colors();
    m_settingsIconLabel = new Core::TintedIconLabel(":/assets/icon/settings.svg", 24,
                                                    colors.textPrimary, messageContainer);
    messageLayout->addWidget(m_settingsIconLabel);

    overlayLayout->addWidget(messageContainer, 0, Qt::AlignCenter);

    overlayLayout->addStretch(3);

    m_deviceNotConfiguredOverlay->hide();
    m_deviceNotConfiguredOverlay->raise();

    // Message selection dialog
    m_selectionDialog = std::make_unique<MessageSelectionDialog>(this);
    connect(this, &LoggingView::dbcConfigChanged, m_selectionDialog.get(),
            &MessageSelectionDialog::setDbcConfig);
    connect(m_selectionDialog.get(), &MessageSelectionDialog::startRequested, this,
            &LoggingView::startRequested);

    applyStyle();
    // ===== Connections =====
    connect(m_btnAction, &QPushButton::clicked, this, [this]() {
        if (m_isRecording)
        {
            emit stopRequested();
        } else
        {
            m_selectionDialog->open();
        }
    });
    connect(this, &LoggingView::detailRequested, this, &LoggingView::onDetailRequested);
}

void LoggingView::setModel(LoggingModel* model)
{
    if (!model)
    {
        return;
    }

    // Disconnect from old model if exists
    if (m_currentModel)
    {
        disconnect(m_currentModel, nullptr, this, nullptr);
    }
    m_currentModel = model;

    m_historyTable->setModel(model);

    // Update empty state visibility based on model data
    connect(model, &QAbstractItemModel::rowsInserted, this, [this, model]() {
        bool isEmpty = model->rowCount() == 0;
        m_emptyLabel->setVisible(isEmpty);
        m_historyTable->setVisible(!isEmpty);
    });

    connect(model, &QAbstractItemModel::rowsRemoved, this, [this, model]() {
        bool isEmpty = model->rowCount() == 0;
        m_emptyLabel->setVisible(isEmpty);
        m_historyTable->setVisible(!isEmpty);
    });

    connect(model, &QAbstractItemModel::modelReset, this, [this, model]() {
        bool isEmpty = model->rowCount() == 0;
        m_emptyLabel->setVisible(isEmpty);
        m_historyTable->setVisible(!isEmpty);
    });

    // Initial state
    bool isEmpty = model->rowCount() == 0;
    m_emptyLabel->setVisible(isEmpty);
    m_historyTable->setVisible(!isEmpty);
}

void LoggingView::showDetailView(QWidget* detailWidget)
{
    if (!detailWidget)
    {
        return;
    }

    // Clear previous detail content
    while (m_detailLayout->count() > 0)
    {
        QLayoutItem* item = m_detailLayout->takeAt(0);
        if (item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Add new detail widget
    m_detailLayout->addWidget(detailWidget);
    detailWidget->setParent(m_detailPage);

    // Switch to detail view
    m_contentStack->setCurrentWidget(m_detailPage);
}

void LoggingView::hideDetailView()
{
    // Switch back to history table
    m_contentStack->setCurrentWidget(m_historyPage);

    // Clear detail content
    while (m_detailLayout->count() > 0)
    {
        QLayoutItem* item = m_detailLayout->takeAt(0);
        if (item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void LoggingView::setRecordingState(bool isRecording)
{
    m_isRecording = isRecording;
    if (isRecording) hideDetailView();
    m_btnAction->setRecordingState(isRecording);
    m_timerLabel->setVisible(isRecording);
}

void LoggingView::updateTimer(qint64 elapsedMs)
{
    m_timerLabel->updateTimer(elapsedMs);
}

void LoggingView::applyStyle()
{
    auto spacing = THEME.spacing();
    auto color = THEME.colors();

    if (m_mainFrame)
    {
        m_mainFrame->setStyleSheet(QString("QFrame#loggingMainFrame {"
                                           "   border: %1px solid %2;"
                                           "   border-radius: %3px;"
                                           "   background-color: %4;"
                                           "}")
                                       .arg(spacing.borderThin)
                                       .arg(color.borderSubtle.name(QColor::HexArgb))
                                       .arg(spacing.radiusMd)
                                       .arg(color.surfaceMain.name(QColor::HexArgb)));
    }

    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setStyleSheet(
            QString("QWidget#deviceNotConfiguredOverlay { "
                    "background-color: rgba(%1, %2, %3, 140); "
                    "}"
                    "QLabel { "
                    "color: %4; "
                    "font-size: %5px; "
                    "}")
                .arg(color.surfaceMain.red())
                .arg(color.surfaceMain.green())
                .arg(color.surfaceMain.blue())
                .arg(color.textPrimary.name())
                .arg(spacing.fontSizeLg));
    }

    if (m_settingsIconLabel)
    {
        m_settingsIconLabel->setColor(color.textPrimary);
    }
}

auto LoggingView::event(QEvent* event) -> bool
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

void LoggingView::showDeviceNotConfiguredOverlay() const
{
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setGeometry(0, 0, width(), height());
        m_deviceNotConfiguredOverlay->show();
        m_deviceNotConfiguredOverlay->raise();
    }
}

void LoggingView::hideDeviceNotConfiguredOverlay() const
{
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->hide();
    }
}

void LoggingView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_deviceNotConfiguredOverlay)
    {
        m_deviceNotConfiguredOverlay->setGeometry(0, 0, width(), height());
    }
}

void LoggingView::onDetailRequested(const QModelIndex& index)
{
    if (!m_currentModel) return;
    auto* detailView = new LoggingDetailView(m_currentModel, index);
    connect(detailView, &LoggingDetailView::backRequested, this, &LoggingView::hideDetailView);
    showDetailView(detailView);
}

}  // namespace Logging
