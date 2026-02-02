#include "logging_view.hpp"

#include <QHeaderView>
#include <QIcon>
#include <QSpacerItem>

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
    m_btnAction = new QPushButton(m_headerBox);
    m_btnAction->setIconSize(QSize(20, 20));
    m_btnAction->setFixedSize(150, 75);
    m_btnAction->setProperty("recording", false);
    m_btnAction->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   border-radius: 30px;"
        "   font-family: 'Roboto';"
        "   font-size: 22px;"
        "   font-weight: 500;"
        "   padding: 10px 10px;"
        "}"
        "QPushButton[recording=\"false\"] {"
        "   background-color: #f3f3f5;"
        "   color: black;"
        "}"
        "QPushButton[recording=\"false\"]:hover {"
        "   background-color: #e8e8ea;"
        "}"
        "QPushButton[recording=\"false\"]:pressed {"
        "   background-color: #d8d8da;"
        "}"
        "QPushButton[recording=\"true\"] {"
        "   background-color: #e85d5d;"
        "   color: white;"
        "}"
        "QPushButton[recording=\"true\"]:hover {"
        "   background-color: #d84848;"
        "}"
        "QPushButton[recording=\"true\"]:pressed {"
        "   background-color: #c83333;"
        "}");

    // Set initial state
    m_btnAction->setText(" Start");
    m_btnAction->setIcon(QIcon(":/assets/icon/logging_start.svg"));
    headerLayout->addWidget(m_btnAction);

    // Timer Label
    m_timerLabel = new QLabel("00:00:00:00", m_headerBox);
    m_timerLabel->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 24px;"
        "   font-weight: 400;"
        "   color: black;"
        "}");
    m_timerLabel->setVisible(false);
    headerLayout->addWidget(m_timerLabel);

    // Status container for message tags (shown during recording)
    m_statusContainer = new QWidget(m_headerBox);
    m_statusLayout = new QHBoxLayout(m_statusContainer);
    m_statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLayout->setSpacing(10);
    m_statusLayout->addStretch();
    m_statusContainer->setVisible(false);
    headerLayout->addWidget(m_statusContainer);

    headerLayout->addStretch();

    mainLayout->addWidget(m_headerBox);

    // ===== Main Frame (bordered container) =====
    m_mainFrame = new QFrame(this);
    m_mainFrame->setFrameShape(QFrame::StyledPanel);
    m_mainFrame->setStyleSheet(
        "QFrame {"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 10px;"
        "   background-color: white;"
        "}");

    auto* frameLayout = new QVBoxLayout(m_mainFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    // ===== Content Stack (History Table <-> Detail View) =====
    m_contentStack = new QStackedWidget(m_mainFrame);

    // --- History Page ---
    m_historyPage = new QWidget(m_contentStack);
    auto* historyLayout = new QVBoxLayout(m_historyPage);
    historyLayout->setContentsMargins(10, 10, 10, 10);

    m_historyTable = new QTreeView(m_historyPage);
    m_historyTable->setRootIsDecorated(false);
    m_historyTable->setAlternatingRowColors(false);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setFrameShape(QFrame::NoFrame);
    m_historyTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_historyTable->setStyleSheet(
        "QTreeView {"
        "   border: none;"
        "   background-color: white;"
        "   font-family: 'Roboto';"
        "   font-size: 20px;"
        "   font-weight: 400;"
        "   padding: 32px;"
        "   outline: none;"
        "}"
        "QTreeView::item {"
        "   height: 61px;"
        "   border: none;"
        "   padding: 5px;"
        "   margin-bottom: 12px;"
        "   background-color: transparent;"
        "}"
        "QTreeView::item:hover {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QTreeView::item:selected {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QTreeView::branch {"
        "   border: none;"
        "}"
        "QHeaderView {"
        "   background-color: white;"
        "   border: none;"
        "}"
        "QHeaderView::section {"
        "   background-color: white;"
        "   border: none;"
        "   border-bottom: 1px solid rgba(0, 0, 0, 0.1);"
        "   padding: 12px 10px;"
        "   font-family: 'Roboto';"
        "   font-size: 24px;"
        "   font-weight: 500;"
        "   color: black;"
        "}");

    // Configure header
    QHeaderView* header = m_historyTable->header();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::Stretch);

    historyLayout->addWidget(m_historyTable);

    // Empty state label
    m_emptyLabel = new QLabel("No past logs", m_historyPage);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(
        "QLabel {"
        "   border: none;"
        "   font-family: 'Roboto';"
        "   font-size: 24px;"
        "   font-weight: 500;"
        "   color: #5a5a5a;"
        "}");
    m_emptyLabel->setVisible(false);
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

    if (isRecording)
    {
        // Recording state - Red Stop button
        m_btnAction->setProperty("recording", true);
        m_btnAction->setText(" Stop");
        m_btnAction->setIcon(QIcon(":/assets/icon/stop_logging.svg"));
        m_timerLabel->setVisible(true);
        m_statusContainer->setVisible(true);
    } else
    {
        // Idle state - Start button
        m_btnAction->setProperty("recording", false);
        m_btnAction->setText(" Start");
        m_btnAction->setIcon(QIcon(":/assets/icon/logging_start.svg"));
        m_timerLabel->setVisible(false);
        m_statusContainer->setVisible(false);
    }

    // Force style refresh
    m_btnAction->style()->unpolish(m_btnAction);
    m_btnAction->style()->polish(m_btnAction);
}

void LoggingView::updateTimer(qint64 elapsedMs)
{
    int hours = elapsedMs / 3600000;
    int minutes = (elapsedMs % 3600000) / 60000;
    int seconds = (elapsedMs % 60000) / 1000;
    int centiseconds = (elapsedMs % 1000) / 10;

    m_timerLabel->setText(QString("%1:%2:%3:%4")
                              .arg(hours, 2, 10, QChar('0'))
                              .arg(minutes, 2, 10, QChar('0'))
                              .arg(seconds, 2, 10, QChar('0'))
                              .arg(centiseconds, 2, 10, QChar('0')));
}

void LoggingView::updateStatusTags(const QStringList& messages)
{
    // Clear existing tags
    while (m_statusLayout->count() > 1)  // Keep the stretch
    {
        QLayoutItem* item = m_statusLayout->takeAt(0);
        if (item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Add new tags
    for (const QString& message : messages)
    {
        auto* tagLabel = new QLabel(message, m_statusContainer);
        tagLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #e2e2e2;"
            "   border-radius: 5px;"
            "   padding: 8px;"
            "   font-family: 'Roboto';"
            "   font-size: 20px;"
            "   font-weight: 400;"
            "   color: black;"
            "}");
        m_statusLayout->insertWidget(m_statusLayout->count() - 1, tagLabel);
    }
}

}  // namespace Logging
