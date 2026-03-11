#include "logging_component.hpp"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <core/event/can_driver_event.hpp>
#include <core/event/dbc_event.hpp>
#include <core/macro/console_logging.hpp>
#include <core/macro/theme.hpp>
#include <ranges>
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

    // View -> Component
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

    // Component -> Model bridge (DBC config updates only)
    connect(this, &LoggingComponent::dbcConfigurationChanged, m_model.get(),
            &LoggingModel::updateDbcConfig);
    connect(this, &LoggingComponent::dbcConfigurationChanged, m_view.get(),
            &LoggingView::dbcConfigChanged);
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
    m_parseSuccessConn = m_eventBroker.subscribe<Core::DBCParsedEvent>(
        [this](const Core::DBCParsedEvent& event) { emit dbcConfigurationChanged(event.config); });

    // Listen for DBC parse errors
    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [](const Core::DBCParseErrorEvent& event) {
            // Error already logged by DBC handler
        });

    m_canDriverChangeConn = m_eventBroker.subscribe<Core::CanDriverChangeEvent>(
        [this](const Core::CanDriverChangeEvent&) {
            QMetaObject::invokeMethod(
                this, [this]() { checkDeviceReadiness(); }, Qt::QueuedConnection);
        });

    m_dbcMsgConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
        [this](const Core::ReceivedCanDbcEvent& event) {
            m_model->onDbcMessageReceived(event.canMessage);
        });

    m_rawMsgConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
        [this](const Core::ReceivedCanRawEvent& event) {
            m_model->onRawMessageReceived(event.canMessage);
        });
    checkDeviceReadiness();
}

// Called when tab becomes inactive - cleans up resources
void LoggingComponent::onStop()
{
    stopLogging();
    m_parseSuccessConn.release();
    m_parseErrorConn.release();
    m_canDriverChangeConn.release();
}

// Initiates a new logging session with user-selected signals
void LoggingComponent::startLogging(LogSessionType logSessionType,
                                    const std::map<uint32_t, QStringList>& selectedSignals)
{
    switch (logSessionType)
    {
        case DBC_BASED: {
            // DBC based logging

            int selectedSignalsCount = 0;
            std::map<uint16_t, std::pair<int, int>> signalsBeforeAfterMessage = {};
            for (const auto& signalList : selectedSignals | std::views::values)
            {
                selectedSignalsCount += signalList.size();
            }
            int currentSignalCount = 0;
            for (const auto& [msgId, signalList] : selectedSignals)
            {
                signalsBeforeAfterMessage[msgId] = {
                    currentSignalCount,
                    selectedSignalsCount - currentSignalCount - signalList.size()};
                currentSignalCount += signalList.size();
            }

            m_model->startNewDbcLogSession(selectedSignals, signalsBeforeAfterMessage);
            m_currentSessionId = m_model->getCurrentSessionId().toStdString();
            m_sessionLogger = Core::LogService::getInstance().getLogger(
                Core::LogContext::CanLogging, m_currentSessionId);
            std::string headerLine;
            headerLine += "Timestamp,";
            for (const auto& [msgId, signalList] : selectedSignals)
            {
                std::string msgName = m_model->getMessageName(msgId).toStdString();
                for (const QString& sig : signalList)
                {
                    headerLine += fmt::format("{}_{}_{}", msgName, sig.toStdString(),
                                              m_model->getSignalUnit(msgId, sig).toStdString());
                    headerLine += ",";
                }
            }
            headerLine.pop_back();
            m_sessionLogger->info(headerLine.c_str());

            // Set recording flag (thread-safe atomic)
            m_isRecording.store(true, std::memory_order_release);

            // Start timer and elapsed time tracking
            m_elapsedTimer.start();
            m_view->updateTimer(0);
            m_timer->start();  // Starts with 100ms interval set in constructor

            break;
        }
        case RAW: {
            m_model->startNewRawLogsSession();
            m_currentSessionId = m_model->getCurrentSessionId().toStdString();
            m_sessionLogger = Core::LogService::getInstance().getLogger(
                Core::LogContext::CanLogging, m_currentSessionId);
            std::string headerLine;
            headerLine += "Timestamp,";
            headerLine += "MessageId,";
            headerLine += "Data";
            m_sessionLogger->info(headerLine.c_str());

            // Set recording flag (thread-safe atomic)
            m_isRecording.store(true, std::memory_order_release);

            // Start timer and elapsed time tracking
            m_elapsedTimer.start();
            m_view->updateTimer(0);
            m_timer->start();  // Starts with 100ms interval set in constructor

            break;
        }
        default: {
        }
    }
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

    // Log session end marker
    if (m_sessionLogger)
    {
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
    stopLogging();
    try
    {
        std::filesystem::copy_file(
            Core::LogService::getLogFilePath(Core::LogContext::CanLogging, sessionId.toStdString()),
            filePath.toStdString());
    } catch (const std::exception& e)
    {
        QMessageBox::critical(m_view.get(), "Error",
                              QString("Failed to export log session: %1").arg(e.what()));
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

// Creates a detail view widget for displaying session information
QWidget* LoggingComponent::createDetailWidget(const LogSession* session)
{
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    auto* detailView = new QWidget(nullptr);
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

void LoggingComponent::checkDeviceReadiness() const
{
    bool isReady = false;
    m_eventBroker.publish<Core::CheckCanDeviceReadyEvent>(Core::CheckCanDeviceReadyEvent(isReady));

    if (isReady == m_lastDeviceReadyState.load(std::memory_order_relaxed))
    {
        return;
    }

    m_lastDeviceReadyState.store(isReady, std::memory_order_relaxed);

    if (isReady)
    {
        m_view->hideDeviceNotConfiguredOverlay();
    } else
    {
        m_view->showDeviceNotConfiguredOverlay();
    }
}

}  // namespace Logging