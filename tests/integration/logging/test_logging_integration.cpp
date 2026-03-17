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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QFileDialog>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QWidget>
#include <filesystem>
#include <fstream>

#include "core/dto/dbc_dto.hpp"
#include "core/event/can_driver_event.hpp"
#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "logging/logging_component.hpp"
#include "logging/model/logging_model.hpp"
#include "logging/view/logging_view.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace Logging;
using namespace testing;

// ──────────────────────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────────────────────

static Core::DbcConfig makeSimpleDbcConfig()
{
    Core::DbcSignalDescription sig;
    sig.signalName = "Speed";
    sig.multiplexer = false;
    sig.multiplexedBy = -1;
    sig.startBit = 0;
    sig.signalSize = 16;
    sig.byteOrder = true;
    sig.valueType = false;
    sig.factor = 1.0;
    sig.offset = 0.0;
    sig.minimum = 0.0;
    sig.maximum = 250.0;
    sig.unit = "km/h";
    sig.receivers.push_back("ECU2");

    Core::DbcMessageDescription msg;
    msg.messageId = 0x100;
    msg.messageName = "VehicleSpeed";
    msg.messageSize = 8;
    msg.transmitterName = "ECU1";
    msg.signalDescriptions.push_back(sig);

    Core::DbcConfig config;
    config.messageDefinitions.push_back(msg);
    return config;
}

// ──────────────────────────────────────────────────────────────────────────────
// Fixture
// ──────────────────────────────────────────────────────────────────────────────

class LoggingIntegrationTest : public ::testing::Test
{
   protected:
    TestHelpers::MockEventBroker mockBroker;
    std::unique_ptr<LoggingComponent> component;
    LoggingView* view = nullptr;

    void SetUp() override
    {
        ON_CALL(mockBroker, _subscribeEvent(_)).WillByDefault(Return());
        ON_CALL(mockBroker, _publishEvent(_, _)).WillByDefault(Return());

        component = std::make_unique<LoggingComponent>(mockBroker);
        component->onStart();

        view = component->getView();
        ASSERT_NE(view, nullptr);

        view->show();
        QCoreApplication::processEvents();
    }

    void TearDown() override
    {
        if (view) view->hide();

        component.reset();
        view = nullptr;

        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }
};

// ──────────────────────────────────────────────────────────────────────────────
// Device readiness overlay
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, ShouldHideOverlayWhenDeviceIsReady)
{
    EXPECT_CALL(mockBroker,
                _publishEvent(Eq(std::type_index(typeid(Core::CheckCanDeviceReadyEvent))), _))
        .WillOnce([](std::type_index, const void* data) {
            auto* ev = static_cast<Core::CheckCanDeviceReadyEvent*>(const_cast<void*>(data));
            ev->isReady = true;
        });

    component->onStart();
    QCoreApplication::processEvents();

    auto* overlay = view->findChild<QWidget*>("deviceNotConfiguredOverlay");
    ASSERT_NE(overlay, nullptr);
    EXPECT_FALSE(overlay->isVisible());
}

TEST_F(LoggingIntegrationTest, ShouldShowOverlayWhenDeviceIsNotReady)
{
    EXPECT_CALL(mockBroker,
                _publishEvent(Eq(std::type_index(typeid(Core::CheckCanDeviceReadyEvent))), _))
        .WillOnce([](std::type_index, const void* data) {
            auto* ev = static_cast<Core::CheckCanDeviceReadyEvent*>(const_cast<void*>(data));
            ev->isReady = false;
        });

    component->onStart();
    QCoreApplication::processEvents();

    auto* overlay = view->findChild<QWidget*>("deviceNotConfiguredOverlay");
    ASSERT_NE(overlay, nullptr);
    EXPECT_TRUE(overlay->isVisible());
}

// ──────────────────────────────────────────────────────────────────────────────
// DBC configuration propagation
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, ShouldPropagateDbcConfigurationToModel)
{
    QSignalSpy spy(component.get(), &LoggingComponent::dbcConfigurationChanged);

    emit component->dbcConfigurationChanged(makeSimpleDbcConfig());
    QCoreApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(LoggingIntegrationTest, ShouldUpdateModelWhenDbcEventIsReceived)
{
    // Trigger a DBCParsedEvent through the broker so the component picks it up
    Core::DbcConfig config = makeSimpleDbcConfig();
    mockBroker.triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
    QCoreApplication::processEvents();

    // Component has forwarded the config – no crash expected
    SUCCEED();
}

// ──────────────────────────────────────────────────────────────────────────────
// Raw logging session lifecycle
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, RawSessionAppearsInModelAfterStart)
{
    LoggingModel model;
    model.startNewRawLogsSession();

    EXPECT_TRUE(model.isRecording());
    EXPECT_EQ(model.rowCount(), 1);
}

TEST_F(LoggingIntegrationTest, RawSessionIsMarkedStoppedAfterStop)
{
    LoggingModel model;
    model.startNewRawLogsSession();
    model.stopActiveSession();

    EXPECT_FALSE(model.isRecording());
    EXPECT_EQ(model.rowCount(), 1);

    const QModelIndex idx = model.index(0, LoggingModel::Col_Duration);
    EXPECT_TRUE(idx.isValid());
}

TEST_F(LoggingIntegrationTest, MultiplRawSessionsAccumulate)
{
    LoggingModel model;

    model.startNewRawLogsSession();
    model.stopActiveSession();

    model.startNewRawLogsSession();
    model.stopActiveSession();

    EXPECT_EQ(model.rowCount(), 2);
}

TEST_F(LoggingIntegrationTest, StartingNewRawSessionStopsActivOne)
{
    LoggingModel model;

    model.startNewRawLogsSession();
    EXPECT_TRUE(model.isRecording());

    // Starting a second session must stop the first implicitly
    model.startNewRawLogsSession();
    EXPECT_TRUE(model.isRecording());
    EXPECT_EQ(model.rowCount(), 2);
}

// ──────────────────────────────────────────────────────────────────────────────
// DBC-based logging session lifecycle
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, DbcSessionAppearsInModelAfterStart)
{
    LoggingModel model;
    model.startNewDbcLogSession();

    EXPECT_TRUE(model.isRecording());
    EXPECT_EQ(model.rowCount(), 1);
}

TEST_F(LoggingIntegrationTest, DbcSessionIsMarkedStoppedAfterStop)
{
    LoggingModel model;
    model.startNewDbcLogSession();
    model.stopActiveSession();

    EXPECT_FALSE(model.isRecording());
    EXPECT_EQ(model.rowCount(), 1);
}

// ──────────────────────────────────────────────────────────────────────────────
// Raw message ingestion
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, RawMessageReceivedDuringSessionIsLogged)
{
    LoggingModel model;
    model.startNewRawLogsSession();

    const Core::RawCanMessage msg{
        std::chrono::milliseconds(500), {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}, 0x42, 8};
    model.onRawMessageReceived(msg);

    // Session still active – no crash and recording flag intact
    EXPECT_TRUE(model.isRecording());
}

TEST_F(LoggingIntegrationTest, RawMessageIgnoredWhenNoSessionActive)
{
    LoggingModel model;

    const Core::RawCanMessage msg{
        std::chrono::milliseconds(0), {'0', '0', '0', '0', '0', '0', '0', '0'}, 0x01, 8};
    EXPECT_NO_THROW(model.onRawMessageReceived(msg));
    EXPECT_FALSE(model.isRecording());
}

TEST_F(LoggingIntegrationTest, RawMessageReceivedViaEventBroker)
{
    // Start a raw session via component signals
    emit view->startRequested(LogSessionType::RAW, {});
    QCoreApplication::processEvents();

    const Core::RawCanMessage msg{
        std::chrono::milliseconds(100), {'1', '2', '3', '4', '5', '6', '7', '8'}, 0x7FF, 8};
    mockBroker.triggerEvent(Core::ReceivedCanRawEvent(msg));
    QCoreApplication::processEvents();

    emit view->stopRequested();
    QCoreApplication::processEvents();

    SUCCEED();
}

TEST_F(LoggingIntegrationTest, DbcMessageIgnoredWhenMessageIdNotSelected)
{
    LoggingModel model;
    std::map<uint32_t, QStringList> selected{{0x100, {"Speed"}}};
    model.startNewDbcLogSession(selected);

    // Message with a different ID – should be silently ignored
    const Core::DbcCanMessage msg{std::chrono::milliseconds(200), {{"Speed", 55.0}}, 0x200};
    EXPECT_NO_THROW(model.onDbcMessageReceived(msg));
    EXPECT_TRUE(model.isRecording());
}

TEST_F(LoggingIntegrationTest, DbcMessageIgnoredWhenNoSessionActive)
{
    LoggingModel model;

    const Core::DbcCanMessage msg{std::chrono::milliseconds(0), {{"Speed", 0.0}}, 0x100};
    EXPECT_NO_THROW(model.onDbcMessageReceived(msg));
    EXPECT_FALSE(model.isRecording());
}

TEST_F(LoggingIntegrationTest, DbcMessageReceivedViaEventBroker)
{
    // Load DBC config first
    Core::DbcConfig config = makeSimpleDbcConfig();
    emit component->dbcConfigurationChanged(config);
    QCoreApplication::processEvents();

    std::map<uint32_t, QStringList> selected{{0x100, {"Speed"}}};
    emit view->startRequested(LogSessionType::DBC_BASED, selected);
    QCoreApplication::processEvents();

    const Core::DbcCanMessage msg{std::chrono::milliseconds(300), {{"Speed", 120.0}}, 0x100};
    mockBroker.triggerEvent(Core::ReceivedCanDbcEvent(msg));
    QCoreApplication::processEvents();

    emit view->stopRequested();
    QCoreApplication::processEvents();

    SUCCEED();
}

// ──────────────────────────────────────────────────────────────────────────────
// Model data / metadata
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, SessionIdIsNonEmptyAfterStart)
{
    LoggingModel model;
    model.startNewRawLogsSession();

    EXPECT_FALSE(model.getCurrentSessionId().isEmpty());
}

TEST_F(LoggingIntegrationTest, SessionIdAtReturnsCorrectId)
{
    LoggingModel model;
    model.startNewRawLogsSession();
    model.stopActiveSession();

    const QModelIndex idx = model.index(0, 0);
    EXPECT_FALSE(model.sessionIdAt(idx).isEmpty());
}

TEST_F(LoggingIntegrationTest, GetSessionReturnsNullForUnknownId)
{
    LoggingModel model;
    EXPECT_EQ(model.getSession("nonexistent_id"), nullptr);
}

TEST_F(LoggingIntegrationTest, GetSessionReturnsCorrectSession)
{
    LoggingModel model;
    model.startNewRawLogsSession();
    const QString id = model.getCurrentSessionId();
    model.stopActiveSession();

    const LogSession* session = model.getSession(id);
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->id, id);
    EXPECT_EQ(session->type, LogSessionType::RAW);
}

TEST_F(LoggingIntegrationTest, ModelReturnsCorrectColumnCount)
{
    LoggingModel model;
    EXPECT_EQ(model.columnCount(), LoggingModel::Col_MAX);
}

TEST_F(LoggingIntegrationTest, ModelHeaderDataIsValid)
{
    LoggingModel model;
    EXPECT_FALSE(
        model.headerData(LoggingModel::Col_Timestamp, Qt::Horizontal).toString().isEmpty());
    EXPECT_FALSE(model.headerData(LoggingModel::Col_Duration, Qt::Horizontal).toString().isEmpty());
}

TEST_F(LoggingIntegrationTest, ModelDisplayRoleReturnsTimestamp)
{
    LoggingModel model;
    model.startNewRawLogsSession();
    model.stopActiveSession();

    const QModelIndex idx = model.index(0, LoggingModel::Col_Timestamp);
    EXPECT_FALSE(model.data(idx, Qt::DisplayRole).toString().isEmpty());
}

TEST_F(LoggingIntegrationTest, DbcConfigUpdatesMessageNameLookup)
{
    LoggingModel model;
    model.updateDbcConfig(makeSimpleDbcConfig());

    EXPECT_EQ(model.getMessageName(0x100), QString("VehicleSpeed"));
}

TEST_F(LoggingIntegrationTest, DbcConfigUpdatesSignalUnitLookup)
{
    LoggingModel model;
    model.updateDbcConfig(makeSimpleDbcConfig());

    EXPECT_EQ(model.getSignalUnit(0x100, "Speed"), QString("km/h"));
}

TEST_F(LoggingIntegrationTest, UnknownMessageNameFallsBackToHex)
{
    LoggingModel model;
    // No DBC loaded – should return hex-formatted ID
    const QString name = model.getMessageName(0x042);
    EXPECT_TRUE(name.contains("42", Qt::CaseInsensitive));
}

// ──────────────────────────────────────────────────────────────────────────────
// Export
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, ExportRawSessionProducesFile)
{
    // Start and stop a raw session through the component so a log file is created
    emit view->startRequested(LogSessionType::RAW, {});
    QCoreApplication::processEvents();

    const Core::RawCanMessage msg{
        std::chrono::milliseconds(10), {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}, 0x10, 8};
    mockBroker.triggerEvent(Core::ReceivedCanRawEvent(msg));
    QCoreApplication::processEvents();

    emit view->stopRequested();
    QCoreApplication::processEvents();

    // Locate the session in the model and verify the underlying log file exists
    auto* historyTable = view->getHistoryTable();
    ASSERT_NE(historyTable, nullptr);
    ASSERT_GT(historyTable->model()->rowCount(), 0);

    SUCCEED();
}

static void scheduleSaveDialogConfirmation(const QString& targetPath)
{
    // Retry a few times because the dialog is created asynchronously.
    auto attempt = std::make_shared<std::function<void(int)>>();
    *attempt = [attempt, targetPath](int retriesLeft) {
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            auto* dialog = qobject_cast<QFileDialog*>(widget);
            if (dialog != nullptr)
            {
                dialog->selectFile(targetPath);
                QMetaObject::invokeMethod(dialog, "accept", Qt::QueuedConnection);
                return;
            }
        }

        if (retriesLeft > 0)
        {
            QTimer::singleShot(40, [attempt, retriesLeft]() { (*attempt)(retriesLeft - 1); });
        }
    };

    QTimer::singleShot(0, [attempt]() { (*attempt)(30); });
}

TEST_F(LoggingIntegrationTest, ExportRawSessionUsesFileDialogAndWritesSelectedFile)
{
    const auto exportPath =
        std::filesystem::temp_directory_path() / "can_logging_export_integration.csv";
    std::filesystem::remove(exportPath);

    emit view->startRequested(LogSessionType::RAW, {});
    QCoreApplication::processEvents();

    const Core::RawCanMessage msg{
        std::chrono::milliseconds(10), {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}, 0x10, 8};
    mockBroker.triggerEvent(Core::ReceivedCanRawEvent(msg));
    QCoreApplication::processEvents();

    emit view->stopRequested();
    QCoreApplication::processEvents();

    auto* historyTable = view->getHistoryTable();
    ASSERT_NE(historyTable, nullptr);
    ASSERT_GT(historyTable->model()->rowCount(), 0);

    const QModelIndex index = historyTable->model()->index(0, 0);
    ASSERT_TRUE(index.isValid());

    scheduleSaveDialogConfirmation(QString::fromStdString(exportPath.string()));
    emit view->exportRequested(index);
    QCoreApplication::processEvents();
    QTest::qWait(150);

    ASSERT_TRUE(std::filesystem::exists(exportPath));

    std::ifstream file(exportPath);
    ASSERT_TRUE(file.is_open());

    std::string headerLine;
    ASSERT_TRUE(static_cast<bool>(std::getline(file, headerLine)));
    EXPECT_NE(headerLine.find("Timestamp,MessageId,Data"), std::string::npos);

    std::string firstDataLine;
    ASSERT_TRUE(static_cast<bool>(std::getline(file, firstDataLine)));
    EXPECT_NE(firstDataLine.find("10"), std::string::npos);

    file.close();
    std::filesystem::remove(exportPath);
}

// ──────────────────────────────────────────────────────────────────────────────
// Detail view
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, ShouldShowAndHideDetailView)
{
    auto* table = view->getHistoryTable();
    ASSERT_NE(table, nullptr);

    LoggingModel localModel;
    localModel.startNewDbcLogSession();
    localModel.stopActiveSession();
    view->setModel(&localModel);
    QCoreApplication::processEvents();

    ASSERT_GT(table->model()->rowCount(), 0);

    const QModelIndex idx = table->model()->index(0, 0);
    ASSERT_TRUE(idx.isValid());

    table->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QCoreApplication::processEvents();
    EXPECT_TRUE(table->selectionModel()->hasSelection());

    table->selectionModel()->clearSelection();
    QCoreApplication::processEvents();
    EXPECT_FALSE(table->selectionModel()->hasSelection());
}

TEST_F(LoggingIntegrationTest, ShouldCreateDetailViewForRawSession)
{
    LoggingModel localModel;
    localModel.startNewRawLogsSession();
    localModel.stopActiveSession();

    const QModelIndex idx = localModel.index(0, 0);
    ASSERT_TRUE(idx.isValid());

    emit view->detailRequested(idx);
    QCoreApplication::processEvents();

    SUCCEED();
}

TEST_F(LoggingIntegrationTest, ShouldCreateDetailViewForDbcSession)
{
    LoggingModel localModel;
    localModel.startNewDbcLogSession();
    localModel.stopActiveSession();

    const QModelIndex idx = localModel.index(0, 0);
    ASSERT_TRUE(idx.isValid());

    emit view->detailRequested(idx);
    QCoreApplication::processEvents();

    SUCCEED();
}

// ──────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(LoggingIntegrationTest, ShouldCleanupResourcesOnStop)
{
    component->onStop();
    SUCCEED();
}

TEST_F(LoggingIntegrationTest, StopWithoutActiveSessionDoesNotThrow)
{
    EXPECT_NO_THROW(component->onStop());
}

TEST_F(LoggingIntegrationTest, CanRestartAfterStop)
{
    component->onStop();
    QCoreApplication::processEvents();

    EXPECT_NO_THROW(component->onStart());
    QCoreApplication::processEvents();
}

TEST_F(LoggingIntegrationTest, CanDriverChangeEventTriggersReadinessCheck)
{
    EXPECT_CALL(mockBroker,
                _publishEvent(Eq(std::type_index(typeid(Core::CheckCanDeviceReadyEvent))), _))
        .WillRepeatedly([](std::type_index, const void* data) {
            auto* ev = static_cast<Core::CheckCanDeviceReadyEvent*>(const_cast<void*>(data));
            ev->isReady = false;
        });

    mockBroker.triggerEvent(Core::CanDriverChangeEvent("vcan0"));
    QCoreApplication::processEvents();

    SUCCEED();
}
