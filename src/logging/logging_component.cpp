#include "logging_component.hpp"

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <core/event/dbc_event.hpp>

#include "core/macro/theme.hpp"
#include "core/widgets/card_widget.hpp"
#

namespace Logging {

// Constructs the logging component and wires up all connections
LoggingComponent::LoggingComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker,
                          "logging",                               // unique internal ID
                          "Logging",                               // tab title
                          QIcon(":/assets/icon/logging/icon.svg")  // tab icon
                          ),
      m_model(std::make_unique<LoggingModel>()),
      m_view(std::make_unique<LoggingView>())
{
    // Initialize the spdlog logger
    Logger::instance().initialize("logs/logging_module.log");
    LOG_INFO("LoggingComponent initialized");
    // Bind model to view
    m_view->setModel(m_model.get());

    // Setup timer for elapsed time tracking with 10ms precision
    m_timer = new QTimer(this);
    m_timer->setInterval(10);  // Update every 10ms for centisecond precision
    connect(m_timer, &QTimer::timeout, this, [this]() {
        qint64 elapsedMs = m_elapsedTimer.elapsed();
        m_view->updateTimer(elapsedMs);
        // Update model duration every second
        if (elapsedMs % 1000 < 10)
        {
            m_model->updateActiveDuration();
        }
    });

    // Delegate (owned)
    m_delegate = std::make_unique<LoggingDelegate>(m_view->getHistoryTable());
    m_view->getHistoryTable()->setItemDelegate(m_delegate.get());

    // Connect delegate signals
    connect(m_delegate.get(), &LoggingDelegate::exportClicked, this,
            [this](const QModelIndex& index) {
                const QString sessionId = m_model->sessionIdAt(index);
                const LogSession* session = m_model->getSession(sessionId);

                // Create a suggested filename
                QString defaultFileName = QString("can_log_%1.csv").arg(sessionId);
                if (session)
                {
                    defaultFileName = QString("can_log_%1_%2.csv")
                                          .arg(session->startDateTime.toString("yyyyMMdd_HHmmss"))
                                          .arg(sessionId);
                }

                const QString filePath = QFileDialog::getSaveFileName(
                    m_view.get(), "Export Log", defaultFileName, "CSV Files (*.csv)");

                if (!filePath.isEmpty())
                {
                    exportLogSession(sessionId, filePath);
                }
            });

    connect(m_delegate.get(), &LoggingDelegate::detailClicked, this,
            &LoggingComponent::onDetailRequested);

    // Message selection dialog
    m_selectionDialog = std::make_unique<MessageSelectionDialog>(m_view.get());

    // View → Component
    connect(m_view.get(), &LoggingView::startRequested, this, &LoggingComponent::startLogging);
    connect(m_view.get(), &LoggingView::stopRequested, this, &LoggingComponent::stopLogging);
    connect(m_view.get(), &LoggingView::detailRequested, this,
            &LoggingComponent::onDetailRequested);

    // Handle logging mode changes
    connect(m_view.get(), &LoggingView::loggingModeChanged, this, [this](bool dbcBased) {
        m_isDbcLogging = dbcBased;
        LOG_INFO("Logging mode changed to: {}", dbcBased ? "DBC-based" : "Raw");
    });

    connect(m_view.get(), &LoggingView::exportRequested, this, [this](const QModelIndex& index) {
        const QString sessionId = m_model->sessionIdAt(index);
        const LogSession* session = m_model->getSession(sessionId);

        // Create a suggested filename
        QString defaultFileName = QString("can_log_%1.csv").arg(sessionId);
        if (session)
        {
            defaultFileName = QString("can_log_%1_%2.csv")
                                  .arg(session->startDateTime.toString("yyyyMMdd_HHmmss"))
                                  .arg(sessionId);
        }

        const QString filePath = QFileDialog::getSaveFileName(m_view.get(), "Export Log",
                                                              defaultFileName, "CSV Files (*.csv)");

        if (!filePath.isEmpty())
        {
            exportLogSession(sessionId, filePath);
        }
    });

    // Component → Model bridges
    connect(this, &LoggingComponent::receiveRawFrame, m_model.get(),
            &LoggingModel::onRawFrameReceived);
    connect(this, &LoggingComponent::receiveDbcSignals, m_model.get(),
            &LoggingModel::onDbcSignalsReceived);
    connect(this, &LoggingComponent::dbcConfigurationChanged, m_model.get(),
            &LoggingModel::updateDbcConfig);

    // Subscribe to DBC events (must be in constructor to receive updates even when tab is inactive)
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            LOG_INFO("Received DBC configuration with {} messages - queuing to UI thread",
                     event.config.messageDefinitions.size());

            // Copy config to avoid dangling references when queued to UI thread
            Core::DbcConfig configCopy = event.config;

            // Use Qt::QueuedConnection to ensure UI updates happen on the main thread
            // This is critical for thread-safety when updating Qt widgets
            QMetaObject::invokeMethod(
                this,
                [this, configCopy]() {
                    emit dbcConfigurationChanged(configCopy);

                    // Update the message selection dialog with the new DBC config
                    // This ensures the dialog always has the latest messages available
                    m_selectionDialog->setDbcConfig(configCopy);
                    LOG_DEBUG("Message selection dialog updated successfully on UI thread");
                },
                Qt::QueuedConnection);
        });

    // Listen for DBC parse errors
    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) {
            LOG_ERROR("DBC parse error occurred: {}", event.errorMessage);
        });

    LOG_INFO("LoggingComponent subscribed to DBC events");
}

LoggingComponent::~LoggingComponent()
{
    m_parseSuccessConn.release();
    m_parseErrorConn.release();
    m_rawMsgConn.release();
    m_dbcMsgConn.release();
    LOG_INFO("LoggingComponent destroyed");
}

// Returns the main logging view widget
auto LoggingComponent::getView() -> QWidget*
{
    // return logging_view
    return m_view.get();
}

// Called when tab becomes active - subscribes to DBC events
void LoggingComponent::onStart()
{
    LOG_INFO("Starting LoggingComponent");
}

// Called when tab becomes inactive - cleans up resources
void LoggingComponent::onStop()
{
    LOG_INFO("Stopping LoggingComponent");
    stopLogging();
    Logger::instance().flush();
}

// Initiates a new logging session with user-selected signals
void LoggingComponent::startLogging()
{
    if (m_model->isRecording())
    {
        LOG_WARN("Attempted to start logging while already recording");
        return;
    }

    LOG_INFO("Initializing logging session");

    // CAN interface is configured in global settings, so we use a descriptive placeholder
    const QString selectedDevice = "Global CAN Interface";
    std::map<uint32_t, QStringList> selectedSignals;

    // Check if DBC-based logging is enabled
    if (m_view->isDbcLoggingEnabled())
    {
        // DBC-based logging: show message selection dialog
        const auto& currentDbc = m_model->getCurrentDbcConfig();

        if (!currentDbc.has_value())
        {
            LOG_WARN("DBC-based logging requested but no valid DBC file is parsed");

            // Create a dialog with a styled card widget
            auto* dialog = new QDialog(m_view.get());
            dialog->setModal(true);
            dialog->setMinimumWidth(400);

            auto* layout = new QVBoxLayout(dialog);
            layout->setContentsMargins(20, 20, 20, 20);
            layout->setSpacing(16);

            // Create warning card
            auto* card =
                new Core::CardWidget("No DBC Loaded",
                                     "DBC-based logging requires a valid DBC file to be loaded.\n"
                                     "Please load a DBC file or switch to raw logging mode.",
                                     ":/assets/icon/dbc_file/load_new.svg", dialog);
            layout->addWidget(card);

            // Add OK button
            auto* okButton = new QPushButton("OK", dialog);
            okButton->setDefault(true);
            connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
            layout->addWidget(okButton, 0, Qt::AlignRight);

            dialog->exec();
            delete dialog;
            return;
        }

        // Ensure dialog is up-to-date before showing
        LOG_DEBUG("Synchronizing message selection dialog with current DBC config ({} messages)",
                  currentDbc.value().messageDefinitions.size());
        m_selectionDialog->setDbcConfig(currentDbc.value());

        // Show message selection dialog
        if (m_selectionDialog->exec() != QDialog::Accepted)
        {
            LOG_DEBUG("User cancelled logging session");
            return;  // User cancelled
        }

        selectedSignals = m_selectionDialog->getSelectedSignals();

        LOG_INFO("Starting new DBC-based logging session on device: {}",
                 selectedDevice.toStdString());
        LOG_INFO("Selected signals for {} messages", selectedSignals.size());

        // Log details of selected signals
        for (const auto& [messageId, signalList] : selectedSignals)
        {
            LOG_DEBUG("Message 0x{:X}: {} signals selected", messageId, signalList.size());
        }
    } else
    {
        // Raw logging: skip dialog and start immediately
        LOG_INFO("Starting new raw logging session on device: {}", selectedDevice.toStdString());
        LOG_INFO("Raw logging mode - no signal selection required");
    }

    m_model->startNewSession(selectedDevice, selectedSignals);

    // Start timer and elapsed time tracking
    m_elapsedTimer.start();
    m_view->updateTimer(0);
    m_timer->start();  // Starts with 10ms interval set in constructor

    // Subscribe based on logging mode
    if (m_isDbcLogging)
    {
        // DBC-based logging: subscribe to decoded messages
        LOG_INFO("Starting DBC-based logging");
        m_dbcMsgConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
            [this](const Core::ReceivedCanDbcEvent& event) {
                LOG_TRACE("Received DBC CAN message - ID: 0x{:X}",
                          static_cast<uint8_t>(event.canMessage.messageId));
                emit receiveDbcSignals(event.canMessage);
            });
    } else
    {
        // Raw logging: subscribe to raw CAN frames
        LOG_INFO("Starting raw logging");
        m_rawMsgConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
            [this](const Core::ReceivedCanRawEvent& event) {
                LOG_TRACE("Received raw CAN frame - ID: 0x{:X}",
                          static_cast<uint8_t>(event.canMessage.messageId));
                emit receiveRawFrame(event.canMessage);
            });
    }

    m_view->setRecordingState(true);
    LOG_INFO("Logging session started successfully");
}

// Stops the active logging session and releases event subscriptions
void LoggingComponent::stopLogging()
{
    if (!m_model->isRecording())
    {
        LOG_WARN("Attempted to stop logging while not recording");
        return;
    }

    LOG_INFO("Stopping logging session");
    m_timer->stop();

    m_rawMsgConn.release();
    m_dbcMsgConn.release();
    m_parseErrorConn.release();
    m_parseSuccessConn.release();

    m_model->stopActiveSession();
    m_view->setRecordingState(false);
    LOG_INFO("Logging session stopped successfully. Duration: {} ms", m_elapsedTimer.elapsed());
}

// Exports a logging session to CSV file
void LoggingComponent::exportLogSession(const QString& sessionId, const QString& filePath)
{
    LOG_INFO("Exporting session {} to {}", sessionId.toStdString(), filePath.toStdString());

    if (!writeToCsv(sessionId, filePath))
    {
        LOG_ERROR("Failed to export session {} to {}", sessionId.toStdString(),
                  filePath.toStdString());
        QMessageBox::critical(m_view.get(), "Export Failed",
                              "Could not write to the specified file.");
    } else
    {
        LOG_INFO("Successfully exported session {} to {}", sessionId.toStdString(),
                 filePath.toStdString());
        QMessageBox::information(
            m_view.get(), "Export Successful",
            QString("Session log exported successfully to:\n%1").arg(filePath));
    }
}

// Handles detail view request for a specific session
void LoggingComponent::onDetailRequested(const QModelIndex& index)
{
    const QString sessionId = m_model->sessionIdAt(index);
    const LogSession* session = m_model->getSession(sessionId);

    if (!session)
    {
        return;
    }

    QWidget* detailWidget = createDetailWidget(session);
    m_view->showDetailView(detailWidget);
}

// Writes session data to CSV file
bool LoggingComponent::writeToCsv(const QString& sessionId, const QString& filePath)
{
    const LogSession* session = m_model->getSession(sessionId);
    if (!session)
    {
        LOG_ERROR("Session not found: {}", sessionId.toStdString());
        return false;
    }

    // Construct the source log file path
    QString sourceLogPath = QString("logs/session_%1.csv").arg(sessionId);
    QFile sourceFile(sourceLogPath);

    // If the session log file exists, copy it to the destination
    if (sourceFile.exists())
    {
        LOG_INFO("Copying log file from {} to {}", sourceLogPath.toStdString(),
                 filePath.toStdString());

        // Remove destination file if it exists
        QFile::remove(filePath);

        // Copy the log file
        if (sourceFile.copy(filePath))
        {
            LOG_INFO("Successfully exported session log to {}", filePath.toStdString());
            return true;
        } else
        {
            LOG_ERROR("Failed to copy log file: {}", sourceFile.errorString().toStdString());
            // Fall through to create metadata-only export
        }
    } else
    {
        LOG_WARN("Session log file not found: {}", sourceLogPath.toStdString());
    }

    // Fallback: Create a metadata-only CSV if log file doesn't exist or copy failed
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_ERROR("Failed to open file for writing: {}", filePath.toStdString());
        return false;
    }

    QTextStream out(&file);
    out << "Session ID,Start Time,Duration,Entry Count,Device\n";
    out << session->id << "," << session->startDateTime.toString(Qt::ISODate) << ","
        << session->duration << "," << session->entryCount << "," << session->deviceName << "\n";

    file.close();
    LOG_INFO("Exported session metadata to {}", filePath.toStdString());
    return true;
}

// Creates a detail view widget for displaying session information
QWidget* LoggingComponent::createDetailWidget(const LogSession* session)
{
    auto* detailView = new QWidget(m_view.get());
    auto* layout = new QVBoxLayout(detailView);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    // ===== Title Section =====
    auto* title = new QLabel(QString("Session Details: %1").arg(session->id), detailView);
    title->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 24px;"
        "   font-weight: 500;"
        "   color: black;"
        "}");

    layout->addWidget(title);

    // ===== Session Information Card =====
    auto* infoCard = new QWidget(detailView);
    infoCard->setStyleSheet(
        "QWidget {"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 10px;"
        "   background-color: white;"
        "   padding: 20px;"
        "}");

    auto* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(20, 20, 20, 20);
    infoLayout->setSpacing(10);

    auto* capturedLabel =
        new QLabel(QString("<b>Captured on:</b> %1")
                       .arg(session->startDateTime.toString("dd.MM.yyyy HH:mm:ss")),
                   infoCard);
    capturedLabel->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 16px;"
        "   color: black;"
        "   border: none;"
        "}");

    auto* messagesLabel =
        new QLabel(QString("<b>Total Messages:</b> %1").arg(session->entryCount), infoCard);
    messagesLabel->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 16px;"
        "   color: black;"
        "   border: none;"
        "}");

    auto* durationLabel =
        new QLabel(QString("<b>Duration:</b> %1").arg(session->duration), infoCard);
    durationLabel->setStyleSheet(
        "QLabel {"
        "   font-family: 'Roboto';"
        "   font-size: 16px;"
        "   color: black;"
        "   border: none;"
        "}");

    infoLayout->addWidget(capturedLabel);
    infoLayout->addWidget(messagesLabel);
    infoLayout->addWidget(durationLabel);

    layout->addWidget(infoCard);
    layout->addStretch();

    // ===== Back Button =====
    auto* backBtn = new QPushButton("Back to History", detailView);
    backBtn->setFixedSize(200, 50);
    backBtn->setStyleSheet(QString("QPushButton {"
                                   "   background-color: %1;"
                                   "   border: none;"
                                   "   border-radius: 25px;"
                                   "   color: black;"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 18px;"
                                   "   font-weight: 500;"
                                   "}"
                                   "QPushButton:hover {"
                                   "   background-color: #e8e8ea;"
                                   "}"
                                   "QPushButton:pressed {"
                                   "   background-color: #d8d8da;"
                                   "}")
                               .arg(THEME.colors().surfaceMain.name()));
    connect(backBtn, &QPushButton::clicked, m_view.get(), &LoggingView::hideDetailView);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(backBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    return detailView;
}

}  // namespace Logging