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

#include "sending_component.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include "can_stream/reader/csv_reader.hpp"
#include "can_stream/reader/mdf4_reader.hpp"
#include "constants.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/lifecycle_event.hpp"
#include "core/macro/console_logging.hpp"
#include "math/service/variable_registry.hpp"
#include "sending_functions.hpp"
#include "view/replay_sending_subview.hpp"

namespace Sending {

SendingComponent::SendingComponent(Core::IEventBroker& broker, Math::VariableRegistry* registry)
    : ITabComponent(broker, QString::fromStdString(Constants::MODULE_IDENTIFIER),
                    Constants::TAB_TITLE, QIcon(Constants::SENDING_ICON_PATH)),
      m_variableRegistry(registry),
      m_model(std::make_unique<SendingModel>()),
      m_view(std::make_unique<SendingView>()),
      m_delegate(new SendingDelegate(this)),
      m_queue(std::make_unique<ScheduledItemQueue>()),
      m_repeatedWorker(std::make_unique<RepeatedProducerWorker>(*m_queue, this)),
      m_replayWorker(std::make_unique<ReplayProducerWorker>(*m_queue, m_eventBroker, this)),
      m_consumerWorker(std::make_unique<SendingConsumerWorker>(*m_queue, this))
{
    m_view->setModel(m_model.get());

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component constructed");
}

SendingComponent::~SendingComponent()
{
    stopReplay();
    stopRepeatedSending();
    m_parseErrorConn.release();
    m_parseSuccessConn.release();
    if (m_view && m_view->parent())
    {
        m_view.release();
    }
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component destroyed");
}

void SendingComponent::onStart()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Starting Sending Component...");

    m_startTime = std::chrono::steady_clock::now();

    setupConnections();
    setupBrokerSubscriptions();

    m_consumerWorker->startConsuming();

    checkDeviceReadiness();
    scanReplaySessions();

    m_eventBroker.publish<Core::ModuleStartedEvent>(
        Core::ModuleStartedEvent(std::type_index(typeid(*this))));
    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component started");
}

void SendingComponent::onStop()
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Stopping Sending Component...");

    stopReplay();
    stopRepeatedSending();

    m_parseErrorConn.release();
    m_parseSuccessConn.release();
    m_canDriverChangeConn.release();

    LOG_INF(Constants::MODULE_IDENTIFIER, "Sending Component stopped");
}

auto SendingComponent::getView() -> QWidget*
{
    return m_view.get();
}

void SendingComponent::onDbcConfigReceived(const Core::DbcConfig& config)
{
    LOG_INF(Constants::MODULE_IDENTIFIER, "Processing DBC config on UI thread");

    m_model->updateDbcConfig(config);
    if (m_variableRegistry)
    {
        m_view->dbcSubView()->populateFromModel(m_model.get(), *m_variableRegistry);
    }
    m_model->buildSendCache();

    LOG_INF(Constants::MODULE_IDENTIFIER, "Created {} message cards",
            config.messageDefinitions.size());
    emit dbcConfigurationChanged(config);
}

void SendingComponent::onDbcParseError() const
{
    m_model->clearEvaluators();
    m_view->dbcSubView()->clearMessages();
}

void SendingComponent::scanReplaySessions()
{
    QDir logsDir("logs");
    const QStringList files = logsDir.entryList({"session_*.mf4", "session_*.csv"}, QDir::Files,
                                                QDir::Name | QDir::Reversed);

    QList<ReplayEntry> entries;
    for (const QString& fileName : files)
    {
        const QString filePath = logsDir.filePath(fileName);
        const bool isCsv = fileName.endsWith(".csv", Qt::CaseInsensitive);

        uint64_t frameCount = 0;
        Core::CanFileType fileType = Core::CanFileType::Raw;

        if (isCsv)
        {
            CanStream::CsvReader reader;
            if (!reader.open(filePath.toStdString())) continue;
            fileType = reader.fileType();
            frameCount = reader.recordCount();
        } else
        {
            CanStream::Mdf4Reader reader;
            if (!reader.open(filePath.toStdString())) continue;
            frameCount = reader.recordCount();
            fileType = reader.fileType();
        }

        // Extract epoch-ms from "session_{epochMs}.{ext}"
        const QString stem = QFileInfo(fileName).completeBaseName();
        const QString epochStr = stem.mid(QString("session_").length());
        bool ok = false;
        const qint64 epochMs = epochStr.toLongLong(&ok);
        const QString displayName =
            ok ? QDateTime::fromMSecsSinceEpoch(epochMs).toString("yyyy-MM-dd HH:mm:ss") : stem;

        entries.append(ReplayEntry{.displayName = displayName,
                                   .filePath = filePath,
                                   .frameCount = frameCount,
                                   .fileType = fileType});
    }

    m_replaySessions = entries;
    m_view->setLogSessions(m_replaySessions);
    LOG_INF(Constants::MODULE_IDENTIFIER, "Found {} replay sessions", m_replaySessions.size());
}

void SendingComponent::addExternalFile(const QString& filePath)
{
    const bool isCsv = filePath.endsWith(".csv", Qt::CaseInsensitive);

    uint64_t frameCount = 0;
    Core::CanFileType fileType = Core::CanFileType::Raw;

    if (isCsv)
    {
        CanStream::CsvReader reader;
        if (!reader.open(filePath.toStdString()))
        {
            LOG_WRN(Constants::MODULE_IDENTIFIER, "Could not open external CSV replay file: {}",
                    filePath.toStdString());
            return;
        }
        fileType = reader.fileType();
        frameCount = reader.recordCount();
    } else
    {
        CanStream::Mdf4Reader reader;
        if (!reader.open(filePath.toStdString()))
        {
            LOG_WRN(Constants::MODULE_IDENTIFIER, "Could not open external replay file: {}",
                    filePath.toStdString());
            return;
        }
        frameCount = reader.recordCount();
        fileType = reader.fileType();
        reader.close();
    }

    const QString displayName = QFileInfo(filePath).fileName();

    // Avoid duplicates by file path
    for (const auto& existing : m_replaySessions)
    {
        if (existing.filePath == filePath)
        {
            return;
        }
    }

    m_replaySessions.append(ReplayEntry{.displayName = displayName,
                                        .filePath = filePath,
                                        .frameCount = frameCount,
                                        .fileType = fileType});

    m_view->setLogSessions(m_replaySessions);
    LOG_INF(Constants::MODULE_IDENTIFIER, "Added external replay file: {} ({} frames)",
            filePath.toStdString(), frameCount);
}

void SendingComponent::publishRawMessageAsync(const Core::RawCanMessage& message) const
{
    Core::RawCanMessage mutableMsg = message;
    if (m_activeManipulationHandler)
    {
        m_activeManipulationHandler->inject(mutableMsg.messageId, mutableMsg.dlc, mutableMsg.data);
    }
    const auto [drop, delayOffset] =
        m_activeManipulationHandler
            ? m_activeManipulationHandler->evaluate()
            : Core::IManipulationHandler::FrameResult{.drop = false, .delayOffset = {}};
    if (drop) return;
    auto context = std::make_shared<RawSendContext>(
        RawSendContext{.broker = &m_eventBroker, .message = mutableMsg});
    m_queue->push(ScheduledItem{.scheduledAt = Clock::now() + delayOffset,
                                .onSend = &rawSendImpl,
                                .context = std::move(context)});
}

void SendingComponent::publishDbcMessageAsync(const Core::DbcCanMessage& message) const
{
    Core::DbcCanMessage mutableMsg = message;
    if (m_activeManipulationHandler)
    {
        m_activeManipulationHandler->inject(mutableMsg);
    }
    Core::RawCanMessage encoded;
    m_eventBroker.publish(Core::EncodeCanMessageDbcEvent(mutableMsg, encoded));
    if (m_activeManipulationHandler)
    {
        m_activeManipulationHandler->inject(encoded.messageId, encoded.dlc, encoded.data);
    }

    const auto [drop, delayOffset] =
        m_activeManipulationHandler
            ? m_activeManipulationHandler->evaluate()
            : Core::IManipulationHandler::FrameResult{.drop = false, .delayOffset = {}};

    if (drop)
    {
        return;
    }
    auto context = std::make_shared<RawSendContext>(
        RawSendContext{.broker = &m_eventBroker, .message = encoded});
    m_queue->push(ScheduledItem{.scheduledAt = Clock::now() + delayOffset,
                                .onSend = &rawSendImpl,
                                .context = std::move(context)});
}

void SendingComponent::setupConnections()
{
    connect(m_model.get(), &SendingModel::requestSendRaw, this,
            [this](const std::string&, const Core::RawCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "Raw send requested: ID=0x{:03X}, DLC={}",
                        message.messageId, message.dlc);
                publishRawMessageAsync(message);
            });

    connect(m_model.get(), &SendingModel::requestSendDbc, this,
            [this](const std::string&, const Core::DbcCanMessage& message) {
                LOG_INF(Constants::MODULE_IDENTIFIER, "DBC send requested: ID=0x{:03X}",
                        message.messageId);
                publishDbcMessageAsync(message);
            });

    connect(m_view.get(), &SendingView::sendOnceRequested, this, &SendingComponent::sendOnce);

    connect(m_view.get(), &SendingView::startRepeatedSendingRequested, this,
            &SendingComponent::startRepeatedSending);
    connect(m_view.get(), &SendingView::stopRepeatedSendingRequested, this,
            &SendingComponent::stopRepeatedSending);

    connect(
        m_consumerWorker.get(), &SendingConsumerWorker::errorOccurred, this,
        [this](const QString& error) {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "Repeated sending error: {}",
                    error.toStdString());
            stopRepeatedSending();

            const auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - m_startTime);

            m_eventBroker.publish<Core::ModuleStoppedEvent>(Core::ModuleStoppedEvent(
                std::type_index(typeid(*this)),
                Core::ModuleDiagnostics{
                    .runtime = runtime, .wasError = true, .errorMessage = error.toStdString()}));
        },
        Qt::QueuedConnection);

    connect(m_view->replaySubView(), &ReplaySendingSubView::externalFileSelected, this,
            [this](const QString& filePath) { addExternalFile(filePath); });

    connect(m_view->replaySubView(), &ReplaySendingSubView::startReplayRequested, this,
            [this](const int index, const double speedFactor) { startReplay(index, speedFactor); });

    connect(m_view->replaySubView(), &ReplaySendingSubView::pauseReplayRequested, this,
            [this]() { pauseReplay(); });

    connect(m_view->replaySubView(), &ReplaySendingSubView::resumeReplayRequested, this,
            [this]() { resumeReplay(); });

    connect(m_view->replaySubView(), &ReplaySendingSubView::stopReplayRequested, this,
            [this]() { stopReplay(); });

    m_replayProgressTimer = new QTimer(this);
    m_replayProgressTimer->setInterval(100);
    connect(m_replayProgressTimer, &QTimer::timeout, this, [this]() {
        m_view->setReplayProgress(m_replayWorker->scheduledFrameCount(), m_replayTotalFrames);
    });

    connect(
        m_replayWorker.get(), &ReplayProducerWorker::replayFinished, this,
        [this]() { m_view->setReplayPlaybackState(ReplaySendingSubView::PlaybackState::Ready); },
        Qt::QueuedConnection);

    connect(
        m_replayWorker.get(), &ReplayProducerWorker::replayStopped, this,
        [this]() {
            m_replayProgressTimer->stop();
            if (m_replayWorker->isRunningReplay())
            {
                return;
            }
            m_view->setReplayProgress(m_replayWorker->scheduledFrameCount(), m_replayTotalFrames);
            m_view->setReplayPlaybackState(m_replaySessions.isEmpty()
                                               ? ReplaySendingSubView::PlaybackState::Disabled
                                               : ReplaySendingSubView::PlaybackState::Ready);
        },
        Qt::QueuedConnection);

    connect(
        m_replayWorker.get(), &ReplayProducerWorker::errorOccurred, this,
        [this](const QString& error) {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "Replay error: {}", error.toStdString());
            m_view->setReplayPlaybackState(ReplaySendingSubView::PlaybackState::Ready);
        },
        Qt::QueuedConnection);
}

void SendingComponent::startRepeatedSending(const int intervalUs) const
{
    if (m_repeatedWorker->isRunning())
    {
        return;
    }

    if (m_variableRegistry)
    {
        m_variableRegistry->reset();
    }
    m_activeManipulationHandler = m_view->dbcSubView()->getManipulationHandler();

    LOG_INF(Constants::MODULE_IDENTIFIER, "Starting repeated sending, interval={}µs", intervalUs);

    m_model->buildSendCache();

    auto callback = [this](const Clock::time_point deadline) -> std::vector<ScheduledItem> {
        std::vector<ScheduledItem> items;
        m_model->forEachCachedMessage(
            [&](Core::RawCanMessage& msg) {
                if (m_activeManipulationHandler)
                {
                    m_activeManipulationHandler->inject(msg.messageId, msg.dlc, msg.data);
                    const auto [drop, delayOffset] = m_activeManipulationHandler->evaluate();
                    if (drop) return;
                    auto context = std::make_shared<RawSendContext>(
                        RawSendContext{.broker = &m_eventBroker, .message = msg});
                    items.push_back(ScheduledItem{.scheduledAt = deadline + delayOffset,
                                                  .onSend = &rawSendImpl,
                                                  .context = std::move(context)});
                } else
                {
                    auto context = std::make_shared<RawSendContext>(
                        RawSendContext{.broker = &m_eventBroker, .message = msg});
                    items.push_back(ScheduledItem{.scheduledAt = deadline,
                                                  .onSend = &rawSendImpl,
                                                  .context = std::move(context)});
                }
            },
            [&](Core::DbcCanMessage& msg) {
                if (m_activeManipulationHandler) m_activeManipulationHandler->inject(msg);
                Core::RawCanMessage encoded;
                m_eventBroker.publish(Core::EncodeCanMessageDbcEvent(msg, encoded));
                if (m_activeManipulationHandler)
                {
                    m_activeManipulationHandler->inject(encoded.messageId, encoded.dlc,
                                                        encoded.data);
                    const auto [drop, delayOffset] = m_activeManipulationHandler->evaluate();
                    if (drop) return;
                    auto context = std::make_shared<RawSendContext>(
                        RawSendContext{.broker = &m_eventBroker, .message = encoded});
                    items.push_back(ScheduledItem{.scheduledAt = deadline + delayOffset,
                                                  .onSend = &rawSendImpl,
                                                  .context = std::move(context)});
                } else
                {
                    auto context = std::make_shared<RawSendContext>(
                        RawSendContext{.broker = &m_eventBroker, .message = encoded});
                    items.push_back(ScheduledItem{.scheduledAt = deadline,
                                                  .onSend = &rawSendImpl,
                                                  .context = std::move(context)});
                }
            });
        return items;
    };

    m_repeatedWorker->startCreating(std::move(callback), intervalUs);
    m_model->setTransmissionStatus(true);
}

void SendingComponent::stopRepeatedSending() const
{
    if (!m_repeatedWorker->isRunning())
    {
        m_model->setTransmissionStatus(false);
        return;
    }

    LOG_INF(Constants::MODULE_IDENTIFIER, "Stopping repeated sending");
    m_repeatedWorker->stopCreating();
    m_model->setTransmissionStatus(false);
}

void SendingComponent::sendOnce() const
{
    if (m_variableRegistry)
    {
        m_variableRegistry->reset();
    }
    m_activeManipulationHandler = m_view->dbcSubView()->getManipulationHandler();
    m_model->transmitCurrent();
}

void SendingComponent::startReplay(const int index, const double speedFactor)
{
    if (index < 0 || index >= m_replaySessions.size())
    {
        return;
    }

    stopRepeatedSending();

    const ReplayEntry entry = m_replaySessions.at(index);

    const bool isCsv = entry.filePath.endsWith(".csv", Qt::CaseInsensitive);
    ReplayProducerWorker::ReaderFactory factory = [path = entry.filePath.toStdString(),
                                                   isCsv]() -> std::unique_ptr<Core::ICanReader> {
        std::unique_ptr<Core::ICanReader> reader;
        if (isCsv)
            reader = std::make_unique<CanStream::CsvReader>();
        else
            reader = std::make_unique<CanStream::Mdf4Reader>();
        if (!reader->open(path)) return nullptr;
        return reader;
    };

    auto manipulationHandler = m_view->replaySubView()->getManipulationHandler();

    m_replayTotalFrames = static_cast<int>(entry.frameCount);

    m_view->setReplayPlaybackState(ReplaySendingSubView::PlaybackState::Running);
    m_view->setReplayProgress(0, m_replayTotalFrames);

    m_replayWorker->startReplay(std::move(factory), entry.frameCount, speedFactor,
                                std::move(manipulationHandler));
    m_replayProgressTimer->start();

    LOG_INF(Constants::MODULE_IDENTIFIER, "Replay started: '{}' at {}x speed",
            entry.filePath.toStdString(), speedFactor);
}

void SendingComponent::pauseReplay()
{
    if (!m_replayWorker || !m_replayWorker->isRunningReplay())
    {
        return;
    }

    m_replayWorker->pauseReplay();
    m_view->setReplayPlaybackState(ReplaySendingSubView::PlaybackState::Paused);
}

void SendingComponent::resumeReplay()
{
    if (!m_replayWorker || !m_replayWorker->isPausedReplay())
    {
        return;
    }

    m_replayWorker->resumeReplay();
    m_view->setReplayPlaybackState(ReplaySendingSubView::PlaybackState::Running);
}

void SendingComponent::stopReplay()
{
    if (m_replayProgressTimer)
    {
        m_replayProgressTimer->stop();
    }

    if (m_replayWorker)
    {
        m_replayWorker->stopReplay();
    }

    m_view->setReplayPlaybackState(m_replaySessions.isEmpty()
                                       ? ReplaySendingSubView::PlaybackState::Disabled
                                       : ReplaySendingSubView::PlaybackState::Ready);
    m_view->setReplayProgress(0, 0);
}

void SendingComponent::setupBrokerSubscriptions()
{
    m_parseSuccessConn =
        m_eventBroker.subscribe<Core::DBCParsedEvent>([this](const Core::DBCParsedEvent& event) {
            LOG_INF(Constants::MODULE_IDENTIFIER, "DBC parse succeeded, queuing to UI thread");
            Core::DbcConfig configCopy = event.config;
            QMetaObject::invokeMethod(
                this, [this, configCopy]() -> void { onDbcConfigReceived(configCopy); },
                Qt::QueuedConnection);
        });

    m_parseErrorConn = m_eventBroker.subscribe<Core::DBCParseErrorEvent>(
        [this](const Core::DBCParseErrorEvent& event) -> void {
            LOG_ERR(Constants::MODULE_IDENTIFIER, "DBC parse failed: {}", event.errorMessage);

            QMetaObject::invokeMethod(this, &SendingComponent::onDbcParseError,
                                      Qt::QueuedConnection);
        });

    m_canDriverChangeConn = m_eventBroker.subscribe<Core::CanDriverChangeEvent>(
        [this](const Core::CanDriverChangeEvent&) -> void {
            QMetaObject::invokeMethod(
                this, [this]() -> void { checkDeviceReadiness(); }, Qt::QueuedConnection);
        });
}

void SendingComponent::checkDeviceReadiness() const
{
    bool isReady = false;
    m_eventBroker.publish<Core::CheckCanDeviceReadyEvent>(Core::CheckCanDeviceReadyEvent(isReady));

    if (isReady == m_lastDeviceReadyState)
    {
        return;
    }

    m_lastDeviceReadyState = isReady;

    if (isReady)
    {
        m_view->hideDeviceNotConfiguredOverlay();
        LOG_INF(Constants::MODULE_IDENTIFIER, "CAN device is ready");
    } else
    {
        m_view->showDeviceNotConfiguredOverlay();
        LOG_WRN(Constants::MODULE_IDENTIFIER, "CAN device not configured");
    }
}

}  // namespace Sending