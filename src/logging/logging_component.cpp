#include "logging_component.hpp"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <core/event/dbc_event.hpp>

namespace Logging {

LoggingComponent::LoggingComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(
          broker,
          "logging",          // unique internal ID
          "Logging",          // tab title
          QIcon()             // icon
      ),
      m_model(std::make_unique<LoggingModel>()),
      m_view(std::make_unique<LoggingView>())
{
    // Bind model to view
    m_view->setModel(m_model.get());

    // Delegate (owned)
    m_delegate = std::make_unique<LoggingDelegate>(m_view->getHistoryTable());
    m_view->getHistoryTable()->setItemDelegate(m_delegate.get());

    // View → Component
    connect(m_view.get(), &LoggingView::startRequested,
            this, &LoggingComponent::startLogging);
    connect(m_view.get(), &LoggingView::stopRequested,
            this, &LoggingComponent::stopLogging);
    connect(m_view.get(), &LoggingView::detailRequested,
            this, &LoggingComponent::onDetailRequested);

    connect(m_view.get(), &LoggingView::exportRequested,
            this, [this](const QModelIndex& index) {
                const QString sessionId = m_model->sessionIdAt(index);
                const QString filePath =
                    QFileDialog::getSaveFileName(
                        m_view.get(),
                        "Export Log",
                        {},
                        "CSV Files (*.csv)");

                if (!filePath.isEmpty()) {
                    exportLogSession(sessionId, filePath);
                }
            });

    // Component → Model bridges
    connect(this, &LoggingComponent::receiveRawFrame,
            m_model.get(), &LoggingModel::onRawFrameReceived);
    connect(this, &LoggingComponent::receiveDbcSignals,
            m_model.get(), &LoggingModel::onDbcSignalsReceived);
    connect(this, &LoggingComponent::dbcConfigurationChanged,
            m_model.get(), &LoggingModel::updateDbcConfig);
}

LoggingComponent::~LoggingComponent() = default;

auto LoggingComponent::getView() -> QWidget*
{
    return m_view.get();
}


void LoggingComponent::onStart()
{
    // Listen for successful DBC parsing
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>(
            [this](const Core::DBCParsedEvent& event) {
                emit dbcConfigurationChanged(event.config);
            });

    // Listen for DBC parse errors
    m_parseErrorConn =
        m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
            [](const Core::DBCParseErrorEvent&) {
                // error handling?
            });
}

void LoggingComponent::onStop()
{
    stopLogging();
    m_parseSuccessConn.release();
    m_parseErrorConn.release();
}

void LoggingComponent::startLogging()
{
    if (m_model->isRecording()) {
        return;
    }

    m_model->startNewSession("Default Device");

    m_rawMsgConn =
        m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
            [this](const Core::ReceivedCanRawEvent& event) {
                emit receiveRawFrame(event.canMessage);
            });

    m_dbcMsgConn =
        m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
            [this](const Core::ReceivedCanDbcEvent& event) {
                emit receiveDbcSignals(event.canMessage);
            });

    m_view->setRecordingState(true);
}

void LoggingComponent::stopLogging()
{
    if (!m_model->isRecording()) {
        return;
    }

    m_rawMsgConn.release();
    m_dbcMsgConn.release();
    m_parseErrorConn.release();
    m_parseSuccessConn.release();

    m_model->stopActiveSession();
    m_view->setRecordingState(false);
}

void LoggingComponent::exportLogSession(const QString& sessionId,
                                       const QString& filePath)
{
    if (!writeToCsv(sessionId, filePath)) {
        QMessageBox::critical(
            m_view.get(),
            "Export Failed",
            "Could not write to the specified file.");
    }
}

void LoggingComponent::onDetailRequested(const QModelIndex& index)
{
    const QString sessionId = m_model->sessionIdAt(index);
    const LogSession* session = m_model->getSession(sessionId);

    if (!session) {
        return;
    }

    QWidget* detailWidget = createDetailWidget(session);
    m_view->showDetailView(detailWidget);
}

bool LoggingComponent::writeToCsv(const QString& sessionId,
                                 const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const LogSession* session = m_model->getSession(sessionId);
    if (!session) {
        return false;
    }

    QTextStream out(&file);
    out << "Session ID,Start Time,Duration,Entry Count\n";
    out << session->id << ","
        << session->startDateTime.toString(Qt::ISODate) << ","
        << session->duration << ","
        << session->entryCount << "\n";

    return true;
}

QWidget* LoggingComponent::createDetailWidget(const LogSession* session)
{
    auto* detailView = new QWidget(m_view.get());
    auto* layout = new QVBoxLayout(detailView);

    auto* title =
        new QLabel(QString("Session Details: %1").arg(session->id), detailView);
    title->setStyleSheet("font-weight: bold; font-size: 16px;");

    auto* info =
        new QLabel(
            QString("Captured on: %1\nTotal Messages: %2")
                .arg(session->startDateTime.toString())
                .arg(session->entryCount),
            detailView);

    auto* backBtn = new QPushButton("Back to History", detailView);
    connect(backBtn, &QPushButton::clicked,
            m_view.get(), &LoggingView::hideDetailView);

    layout->addWidget(title);
    layout->addWidget(info);
    layout->addStretch();
    layout->addWidget(backBtn);

    return detailView;
}

} // namespace Logging