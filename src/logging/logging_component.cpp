#include "logging_component.hpp"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <core/event/dbc_event.hpp>
#include <core/macro/console_logging.hpp>
#include <core/macro/theme.hpp>
#include <set>

#include "constants.hpp"

namespace Logging {

// Constructs the logging component and wires up all connections
LoggingComponent::LoggingComponent(Core::IEventBroker& broker)
    : Core::ITabComponent(broker, "logging", "Logging", QIcon(Constants::LOGGING_ICON_PATH)),
      m_model(std::make_unique<LoggingModel>()),
      m_view(std::make_unique<LoggingView>())
{
    // Bind model to view
    m_view->setModel(m_model.get());

    // Setup timer for elapsed time tracking (100ms for smooth display)
    m_timer = new QTimer(this);
    m_timer->setInterval(100);  // Update every 100ms for smooth timer
    connect(m_timer, &QTimer::timeout, this, [this]() {
        qint64 elapsedMs = m_elapsedTimer.elapsed();
        m_view->updateTimer(elapsedMs);

        // Update model duration only every second to avoid excessive repaints
        if (elapsedMs % 1000 < 100)
        {
            m_model->updateActiveDuration();
        }
    });

    // Delegate (owned)
    m_delegate = std::make_unique<LoggingDelegate>(m_view->getHistoryTable());
    m_view->getHistoryTable()->setItemDelegate(m_delegate.get());

    // Connect delegate signals
    connect(
        m_delegate.get(), &LoggingDelegate::exportClicked, this, [this](const QModelIndex& index) {
            const QString sessionId = m_model->sessionIdAt(index);
            const QString filePath =
                QFileDialog::getSaveFileName(m_view.get(), "Export Log", {}, "CSV Files (*.csv)");

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

    connect(m_view.get(), &LoggingView::exportRequested, this, [this](const QModelIndex& index) {
        const QString sessionId = m_model->sessionIdAt(index);
        const QString filePath =
            QFileDialog::getSaveFileName(m_view.get(), "Export Log", {}, "CSV Files (*.csv)");

        if (!filePath.isEmpty())
        {
            exportLogSession(sessionId, filePath);
        }
    });

    // Component → Model bridge (DBC config updates only)
    connect(this, &LoggingComponent::dbcConfigurationChanged, m_model.get(),
            &LoggingModel::updateDbcConfig);
}

LoggingComponent::~LoggingComponent() = default;

// Returns the main logging view widget
auto LoggingComponent::getView() -> QWidget*
{
    // return logging_view
    return m_view.get();
}

// Called when tab becomes active - subscribes to DBC events
void LoggingComponent::onStart()
{
    // Listen for successful DBC parsing
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            emit dbcConfigurationChanged(event.config);

            // Update the message selection dialog with the new DBC config
            m_selectionDialog->setDbcConfig(event.config);
        });

    // Listen for DBC parse errors
    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) {
            // Error already logged by DBC handler
        });
}

// Called when tab becomes inactive - cleans up resources
void LoggingComponent::onStop()
{
    stopLogging();
    m_parseSuccessConn.release();
    m_parseErrorConn.release();
}

// Initiates a new logging session with user-selected signals
void LoggingComponent::startLogging()
{
    if (m_model->isRecording())
    {
        return;
    }

    // Show message selection dialog
    if (m_selectionDialog->exec() != QDialog::Accepted)
    {
        return;  // User cancelled
    }

    std::map<uint32_t, QStringList> selectedSignals = m_selectionDialog->getSelectedSignals();

    // Populate thread-safe caches (for CAN thread access)
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_selectedSignalsCache.clear();
        m_messageNamesCache.clear();
        m_signalUnitsCache.clear();

        for (const auto& [msgId, signalList] : selectedSignals)
        {
            // Convert QStringList to std::vector<std::string>
            std::vector<std::string> signalNames;
            signalNames.reserve(signalList.size());
            for (const QString& sig : signalList)
            {
                signalNames.push_back(sig.toStdString());
            }
            m_selectedSignalsCache[msgId] = std::move(signalNames);

            // Cache message name
            m_messageNamesCache[msgId] = m_model->getMessageName(msgId).toStdString();

            // Cache signal units
            for (const QString& sig : signalList)
            {
                QString unit = m_model->getSignalUnit(msgId, sig);
                uint64_t hash = (static_cast<uint64_t>(msgId) << 32) |
                                std::hash<std::string>{}(sig.toStdString());
                m_signalUnitsCache[hash] = unit.toStdString();
            }
        }
    }

    // Start new session in model (no device name needed)
    m_model->startNewSession(QString(), selectedSignals);

    // Get session ID and create session logger
    m_currentSessionId = m_model->getCurrentSessionId().toStdString();
    m_sessionLogger =
        Core::LogService::getInstance().getLogger(Core::LogContext::CanLogging, m_currentSessionId);

    // Log session start marker
    m_sessionLogger->info("SESSION_START,{}", selectedSignals.size());

    // Set recording flag (thread-safe atomic)
    m_isRecording.store(true, std::memory_order_release);

    // Start timer and elapsed time tracking
    m_elapsedTimer.start();
    m_view->updateTimer(0);
    m_timer->start();  // Starts with 100ms interval set in constructor

    // Subscribe to raw CAN frames (for messages without DBC definitions)
    m_rawMsgConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
        [this](const Core::ReceivedCanRawEvent& event) {
            // Fast path: early return if not recording (thread-safe atomic)
            if (!m_isRecording.load(std::memory_order_acquire) || !m_sessionLogger) [[unlikely]]
            {
                return;
            }

            const auto& msg = event.canMessage;

            // Check if this message has a DBC definition (thread-safe cache lookup)
            std::string messageName;
            {
                std::lock_guard<std::mutex> lock(m_cacheMutex);
                auto it = m_messageNamesCache.find(msg.messageId);
                if (it != m_messageNamesCache.end())
                {
                    messageName = it->second;
                } else
                {
                    messageName = "UNKNOWN_0x" + fmt::format("{:03X}", msg.messageId);
                }
            }

            // Only log raw if no DBC definition exists
            if (messageName.find("UNKNOWN_") == 0)
            {
                // Optimized hex formatting - pre-allocate space
                std::string hexData;
                hexData.reserve(msg.dlc * 3);
                for (int i = 0; i < msg.dlc; ++i)
                {
                    hexData += fmt::format("{:02X}", static_cast<unsigned char>(msg.data[i]));
                    if (i < msg.dlc - 1) hexData += " ";
                }

                // Async log (non-blocking, thread-safe)
                m_sessionLogger->info("0x{:03X},{},RAW_DATA,{},hex", msg.messageId, messageName,
                                      hexData);
            }
        });

    // Subscribe to decoded DBC messages
    m_dbcMsgConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
        [this](const Core::ReceivedCanDbcEvent& event) {
            // Fast path: early return if not recording (thread-safe atomic)
            if (!m_isRecording.load(std::memory_order_acquire) || !m_sessionLogger) [[unlikely]]
            {
                return;
            }

            const auto& msg = event.canMessage;

            // Get selected signals (thread-safe cache)
            std::vector<std::string> selectedSignals;
            std::string messageName;
            {
                std::lock_guard<std::mutex> lock(m_cacheMutex);

                auto it = m_selectedSignalsCache.find(msg.messageId);
                if (it == m_selectedSignalsCache.end())
                {
                    return;  // No signals selected for this message ID
                }
                selectedSignals = it->second;

                // Get message name
                auto nameIt = m_messageNamesCache.find(msg.messageId);
                messageName = (nameIt != m_messageNamesCache.end()) ? nameIt->second : "UNKNOWN";
            }

            // Iterate through signal values and log selected ones (no Qt objects, thread-safe!)
            for (const auto& signal : msg.signalValues)
            {
                // Check if signal is in selected list
                auto it = std::find(selectedSignals.begin(), selectedSignals.end(), signal.name);
                if (it != selectedSignals.end())
                {
                    // Get signal unit from cache
                    uint64_t hash = (static_cast<uint64_t>(msg.messageId) << 32) |
                                    std::hash<std::string>{}(signal.name);
                    std::string unit;
                    {
                        std::lock_guard<std::mutex> lock(m_cacheMutex);
                        auto unitIt = m_signalUnitsCache.find(hash);
                        unit = (unitIt != m_signalUnitsCache.end()) ? unitIt->second : "";
                    }

                    // Async log (non-blocking, thread-safe)
                    m_sessionLogger->info("0x{:03X},{},{},{:.6f},{}", msg.messageId, messageName,
                                          signal.name, signal.value, unit);
                }
            }
        });

    m_view->setRecordingState(true);
}

// Stops the active logging session and releases event subscriptions
void LoggingComponent::stopLogging()
{
    if (!m_model->isRecording())
    {
        return;
    }

    // Clear recording flag first (thread-safe atomic)
    m_isRecording.store(false, std::memory_order_release);

    m_timer->stop();

    // Release event subscriptions (only CAN message subscriptions, NOT parse connections)
    m_rawMsgConn.release();
    m_dbcMsgConn.release();

    // Clear thread-safe caches
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_selectedSignalsCache.clear();
        m_messageNamesCache.clear();
        m_signalUnitsCache.clear();
    }

    // Log session end marker
    if (m_sessionLogger)
    {
        m_sessionLogger->info("SESSION_END,{}", m_elapsedTimer.elapsed());
        m_sessionLogger->flush();

        // Close session logger
        Core::LogService::getInstance().closeLogger(Core::LogContext::CanLogging,
                                                    m_currentSessionId);
        m_sessionLogger = nullptr;
    }

    m_model->stopActiveSession();
    m_view->setRecordingState(false);
}

// Exports a logging session to CSV file
void LoggingComponent::exportLogSession(const QString& sessionId, const QString& filePath)
{
    if (!writeToCsv(sessionId, filePath))
    {
        QMessageBox::critical(m_view.get(), "Export Failed",
                              "Could not write to the specified file.");
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

// Writes session data from log file to CSV with progress dialog
bool LoggingComponent::writeToCsv(const QString& sessionId, const QString& filePath)
{
    // Input log file path
    QString logFilePath = QString("logs/session_%1_CanLogging.log").arg(sessionId);

    QFile logFile(logFilePath);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    // Output CSV file
    QFile csvFile(filePath);
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    // Count total lines for progress
    QTextStream counter(&logFile);
    int totalLines = 0;
    while (!counter.atEnd())
    {
        counter.readLine();
        totalLines++;
    }
    logFile.seek(0);  // Reset to beginning

    // Create progress dialog
    QProgressDialog progress("Exporting to CSV...", "Cancel", 0, totalLines, m_view.get());
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);  // Show after 500ms

    QTextStream in(&logFile);
    QTextStream out(&csvFile);

    // Write CSV header
    out << "Timestamp,Log Level,Message ID,Message Name,Signal Name,Value,Unit\n";

    int lineCount = 0;
    bool hadErrors = false;

    // Parse each log line
    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineCount++;

        // Update progress every 100 lines
        if (lineCount % 100 == 0)
        {
            progress.setValue(lineCount);
            if (progress.wasCanceled())
            {
                return false;
            }
        }

        // Skip empty lines
        if (line.trimmed().isEmpty())
        {
            continue;
        }

        // Parse format: "2024-01-15 14:23:01.123|info|0x123,EngineSpeed,RPM,2500.123,rpm"
        QStringList parts = line.split('|');
        if (parts.size() >= 3)
        {
            QString timestamp = parts[0];
            QString level = parts[1];
            QString data = parts[2];

            // Special handling for session markers
            if (data.startsWith("SESSION_START") || data.startsWith("SESSION_END"))
            {
                out << timestamp << "," << level << "," << data << ",,,\n";
            } else
            {
                // Write as CSV (data already in correct format)
                out << timestamp << "," << level << "," << data << "\n";
            }
        } else
        {
            hadErrors = true;
        }
    }

    progress.setValue(totalLines);

    logFile.close();
    csvFile.close();

    return true;
}

// Creates a detail view widget for displaying session information
QWidget* LoggingComponent::createDetailWidget(const LogSession* session)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* detailView = new QWidget(m_view.get());
    auto* layout = new QVBoxLayout(detailView);
    layout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                               spacing.spacingLg);
    layout->setSpacing(spacing.spacingMd);

    // ===== Title Section =====
    auto* title = new QLabel(QString("Session Details: %1").arg(session->id), detailView);
    const QString titleStyle = QString(
                                   "QLabel {"
                                   "   font-size: %3px;"
                                   "   font-weight: %1;"
                                   "   color: %2;"
                                   "}")
                                   .arg(spacing.fontWeightMedium)
                                   .arg(colors.textPrimary.name())
                                   .arg(spacing.fontSizeLg);
    title->setStyleSheet(titleStyle);

    layout->addWidget(title);

    // ===== Session Information Card =====
    auto* infoCard = new QWidget(detailView);
    const QString cardStyle = QString(
                                  "QWidget {"
                                  "   border: %1px solid %2;"
                                  "   border-radius: %3px;"
                                  "   background-color: %4;"
                                  "   padding: %5px;"
                                  "}")
                                  .arg(spacing.borderThin)
                                  .arg(colors.borderSubtle.name())
                                  .arg(spacing.radiusMd)
                                  .arg(colors.surfaceMain.name())
                                  .arg(spacing.spacingLg);
    infoCard->setStyleSheet(cardStyle);

    auto* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(spacing.spacingLg, spacing.spacingLg, spacing.spacingLg,
                                   spacing.spacingLg);
    infoLayout->setSpacing(spacing.spacingSm);

    const QString labelStyle = QString(
                                   "QLabel {"
                                   "   font-family: 'Roboto';"
                                   "   font-size: 16px;"
                                   "   color: %1;"
                                   "   border: none;"
                                   "}")
                                   .arg(colors.textPrimary.name());

    auto* capturedLabel =
        new QLabel(QString("<b>Captured on:</b> %1")
                       .arg(session->startDateTime.toString("dd.MM.yyyy HH:mm:ss")),
                   infoCard);
    capturedLabel->setStyleSheet(labelStyle);

    auto* durationLabel =
        new QLabel(QString("<b>Duration:</b> %1").arg(session->duration), infoCard);
    durationLabel->setStyleSheet(labelStyle);

    auto* logFileLabel = new QLabel(
        QString("<b>Log File:</b> logs/session_%1_CanLogging.log").arg(session->id), infoCard);
    logFileLabel->setStyleSheet(labelStyle);

    infoLayout->addWidget(capturedLabel);
    infoLayout->addWidget(durationLabel);
    infoLayout->addWidget(logFileLabel);

    layout->addWidget(infoCard);
    layout->addStretch();

    // ===== Back Button =====
    auto* backBtn = new QPushButton("Back to History", detailView);
    backBtn->setFixedSize(200, 50);
    const QString btnStyle = QString(
                                 "QPushButton {"
                                 "   background-color: %1;"
                                 "   border: none;"
                                 "   border-radius: 25px;"
                                 "   color: %2;"
                                 "   font-family: 'Roboto';"
                                 "   font-size: %3px;"
                                 "   font-weight: %4;"
                                 "}"
                                 "QPushButton:hover {"
                                 "   background-color: %5;"
                                 "}"
                                 "QPushButton:pressed {"
                                 "   background-color: %5;"
                                 "}")
                                 .arg(colors.surfacePrimary.name())
                                 .arg(colors.textPrimary.name())
                                 .arg(spacing.fontSizeLg)
                                 .arg(spacing.fontWeightMedium)
                                 .arg(colors.surfaceHover.name());
    backBtn->setStyleSheet(btnStyle);
    connect(backBtn, &QPushButton::clicked, m_view.get(), &LoggingView::hideDetailView);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(backBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    return detailView;
}

}  // namespace Logging