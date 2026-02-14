#include "logging_view.hpp"

#include <QHeaderView>
#include <QIcon>
#include <QSpacerItem>

#include "core/macro/theme.hpp"
#include "core/theme/style_event.hpp"

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

    applyStyle();
    // ===== Connections =====
    connect(m_btnAction, &QPushButton::clicked, this, [this]() {
        if (m_isRecording)
        {
            emit stopRequested();
        } else
        {
            emit startRequested();
        }
    });
}

void LoggingView::setModel(LoggingModel* model)
{
    if (!model)
    {
        return;
    }

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
            item->widget()->setParent(nullptr);
        }
        delete item;
    }

    // Add new detail widget
    m_detailLayout->addWidget(detailWidget);

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
            item->widget()->setParent(nullptr);
        }
        delete item;
    }
}

void LoggingView::setRecordingState(bool isRecording)
{
    m_isRecording = isRecording;

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
        m_mainFrame->setStyleSheet(QString("QFrame {"
                                           "   border: %1px solid %2;"
                                           "   border-radius: %3px;"
                                           "   background-color: %4;"
                                           "}")
                                       .arg(spacing.borderThin)
                                       .arg(color.borderSubtle.name(QColor::HexArgb))
                                       .arg(spacing.radiusMd)
                                       .arg(color.surfaceMain.name(QColor::HexArgb)));
    }
}

bool LoggingView::event(QEvent* event)
{
    if (event->type() == Core::StyleEvent::EventType)
    {
        applyStyle();
        return true;
    }
    return QWidget::event(event);
}

}  // namespace Logging
