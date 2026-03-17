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

    connect(m_delegate.get(), &LoggingDelegate::detailClicked, m_view.get(),
            &LoggingView::onDetailRequested);

    // View -> Component
    connect(m_view.get(), &LoggingView::startRequested, this, &LoggingComponent::startLogging);
    connect(m_view.get(), &LoggingView::stopRequested, this, &LoggingComponent::stopLogging);

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

LoggingComponent::~LoggingComponent()
{
    if (m_view && m_view->parent())
    {
        m_view.release();
    }
}

// Returns the main logging view widget
auto LoggingComponent::getView() -> LoggingView*
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
            m_timer->start();

            m_dbcMsgConn = m_eventBroker.subscribe<Core::ReceivedCanDbcEvent>(
                [this](const Core::ReceivedCanDbcEvent& event) {
                    m_model->onDbcMessageReceived(event.canMessage);
                });

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
            m_timer->start();

            m_rawMsgConn = m_eventBroker.subscribe<Core::ReceivedCanRawEvent>(
                [this](const Core::ReceivedCanRawEvent& event) {
                    m_model->onRawMessageReceived(event.canMessage);
                });

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