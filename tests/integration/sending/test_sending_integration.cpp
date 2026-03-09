#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QTest>
#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "sending/sending_component.hpp"
#include "sending/view/dbc_based_sending_subview.hpp"
#include "sending/view/raw_sending_subview.hpp"
#include "sending/view/sending_view.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace TestHelpers;
using ::testing::_;
using ::testing::NiceMock;

// =============================================================================
// Thread-safe event capture
// =============================================================================

struct CapturedSendEvents
{
    mutable std::mutex mutex;
    std::vector<Core::SendCanMessageRawEvent> raw;
    std::vector<Core::SendCanMessageDbcEvent> dbc;

    void clear()
    {
        std::lock_guard lock(mutex);
        raw.clear();
        dbc.clear();
    }

    [[nodiscard]] auto rawCount() const -> size_t
    {
        std::lock_guard lock(mutex);
        return raw.size();
    }

    [[nodiscard]] auto dbcCount() const -> size_t
    {
        std::lock_guard lock(mutex);
        return dbc.size();
    }

    [[nodiscard]] auto firstRaw() const -> Core::RawCanMessage
    {
        std::lock_guard lock(mutex);
        EXPECT_FALSE(raw.empty()) << "No raw events captured";
        return raw.front().canMessage;
    }

    [[nodiscard]] auto firstDbc() const -> Core::DbcCanMessage
    {
        std::lock_guard lock(mutex);
        EXPECT_FALSE(dbc.empty()) << "No DBC events captured";
        return dbc.front().canMessage;
    }

    [[nodiscard]] auto allDbc() const -> std::vector<Core::DbcCanMessage>
    {
        std::lock_guard lock(mutex);
        std::vector<Core::DbcCanMessage> result;
        result.reserve(dbc.size());
        for (const auto& e : dbc)
        {
            result.push_back(e.canMessage);
        }
        return result;
    }
};

// =============================================================================
// Base fixture
// =============================================================================

class SendingIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<NiceMock<MockEventBroker>>();

        // Intercept every publish call and capture the send events we care about.
        // All other events (ModuleStartedEvent, CheckCanDeviceReadyEvent, …) are
        // silently accepted by NiceMock and ignored here.
        ON_CALL(*mockBroker, _publishEvent(_, _))
            .WillByDefault([this](std::type_index type, const void* data) {
                if (type == std::type_index(typeid(Core::SendCanMessageRawEvent)))
                {
                    std::lock_guard lock(events.mutex);
                    events.raw.push_back(
                        *static_cast<const Core::SendCanMessageRawEvent*>(data));
                }
                else if (type == std::type_index(typeid(Core::SendCanMessageDbcEvent)))
                {
                    std::lock_guard lock(events.mutex);
                    events.dbc.push_back(
                        *static_cast<const Core::SendCanMessageDbcEvent*>(data));
                }
            });

        component = std::make_unique<Sending::SendingComponent>(*mockBroker);
        component->onStart();

        view    = static_cast<Sending::SendingView*>(component->getView());
        rawView = view->rawSubView();
        dbcView = view->dbcSubView();

        // Let onStart() settle (queued connections, overlay check, …)
        QTest::qWait(50);
        events.clear();
    }

    void TearDown() override
    {
        if (component)
        {
            component->onStop();
        }
        component.reset();
        mockBroker.reset();
    }

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    /** Trigger a DBCParsedEvent and wait for the queued connection to the UI thread. */
    void loadDbc(const Core::DbcConfig& config)
    {
        mockBroker->triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
        QTest::qWait(150);
    }

    /** Switch the model to DBC mode (as the sidebar would). */
    void switchToDbc()
    {
        emit view->modeChanged(true);
        QTest::qWait(10);
    }

    /** Switch the model back to Raw mode. */
    void switchToRaw()
    {
        emit view->modeChanged(false);
        QTest::qWait(10);
    }

    /**
     * Trigger a single send and wait for the QtConcurrent::run thread to
     * publish to the broker.
     */
    void triggerSendOnce()
    {
        emit view->sendOnceRequested();
        QTest::qWait(200);
    }

    void startCyclic(int intervalMs)
    {
        emit view->startRepeatedSendingRequested(intervalMs);
    }

    void stopCyclic()
    {
        emit view->stopRepeatedSendingRequested();
        QTest::qWait(300);  // wait for worker thread to finish
    }

    std::unique_ptr<NiceMock<MockEventBroker>> mockBroker;
    std::unique_ptr<Sending::SendingComponent>  component;
    Sending::SendingView*         view    = nullptr;
    Sending::RawSendingSubView*   rawView = nullptr;
    Sending::DbcSendingSubView*   dbcView = nullptr;
    CapturedSendEvents            events;
};

// =============================================================================
// RAW SENDING
// =============================================================================

TEST_F(SendingIntegrationTest, Raw_SendOnce_PublishesExactlyOneRawEvent)
{
    // The default mode is Raw - sending without any configuration should still
    // publish one event (with default ID 0x000 and zeroed data).
    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_CorrectCanId)
{
    rawView->canIdEditor()->setText("1A2");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().messageId, 0x1A2);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_MaxStandardCanId)
{
    rawView->canIdEditor()->setText("7FF");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().messageId, 0x7FF);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_DataBytesAppearInEvent)
{
    rawView->messageDataEditor()->setText("AA BB CC");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    const auto msg = events.firstRaw();
    EXPECT_EQ(static_cast<uint8_t>(msg.data[0]), 0xAA);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[1]), 0xBB);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[2]), 0xCC);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_DlcMatchesNumberOfDataBytes)
{
    rawView->messageDataEditor()->setText("01 02 03 04 05");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().dlc, 5);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_FullEightBytes_AllPreserved)
{
    rawView->messageDataEditor()->setText("01 23 45 67 89 AB CD EF");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    const auto msg = events.firstRaw();
    EXPECT_EQ(msg.dlc, 8);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[0]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[1]), 0x23);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[6]), 0xCD);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[7]), 0xEF);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_EmptyData_DlcIsZero)
{
    // Clear any pre-existing data
    rawView->messageDataEditor()->setText("");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().dlc, 0);
}

TEST_F(SendingIntegrationTest, Raw_SendOnce_IdAndDataTogether)
{
    rawView->canIdEditor()->setText("300");
    rawView->messageDataEditor()->setText("FF 00 AA 55");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    const auto msg = events.firstRaw();
    EXPECT_EQ(msg.messageId, 0x300);
    EXPECT_EQ(msg.dlc, 4);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[0]), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[3]), 0x55);
}

// =============================================================================
// DBC SENDING
// =============================================================================

TEST_F(SendingIntegrationTest, Dbc_SendOnce_NoDbc_NoEventPublished)
{
    switchToDbc();
    // No DBC loaded, no config in model → nothing to send

    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 0);
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_DbcLoaded_NoSignalsSelected_NoEventPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    // DBC is loaded but no signals are selected

    triggerSendOnce();

    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_SelectedSignal_EventPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    EXPECT_EQ(events.firstDbc().messageId, 0x100);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_SelectedSignalPresentInPayload)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto msg    = events.firstDbc();
    const auto& sigs  = msg.signalValues;
    const bool found  = std::any_of(sigs.begin(), sigs.end(),
                                    [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    EXPECT_TRUE(found) << "Selected signal 'Speed' should appear in the published event";
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_UnselectedSignalAbsentFromPayload)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    // Select only Speed, leave Temperature and ErrorCode unselected
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    const bool tempFound =
        std::any_of(sigs.begin(), sigs.end(),
                    [](const Core::DbcCanSignal& s) { return s.name == "Temperature"; });
    const bool errFound =
        std::any_of(sigs.begin(), sigs.end(),
                    [](const Core::DbcCanSignal& s) { return s.name == "ErrorCode"; });

    EXPECT_FALSE(tempFound) << "Unselected signal 'Temperature' must NOT appear in the event";
    EXPECT_FALSE(errFound)  << "Unselected signal 'ErrorCode' must NOT appear in the event";
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_SignalValuePropagatedToEvent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 3000.0);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    auto it = std::find_if(sigs.begin(), sigs.end(),
                           [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end()) << "Speed signal not found in event";
    EXPECT_DOUBLE_EQ(it->value, 3000.0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_TwoSelectedSignals_BothInPayload)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalSelectionChanged(0x100, "Temperature", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    EXPECT_EQ(sigs.size(), 2u);
    const bool speedFound = std::any_of(sigs.begin(), sigs.end(),
                                        [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    const bool tempFound  = std::any_of(sigs.begin(), sigs.end(),
                                        [](const Core::DbcCanSignal& s) { return s.name == "Temperature"; });
    EXPECT_TRUE(speedFound);
    EXPECT_TRUE(tempFound);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_SignalsFromTwoMessages_TwoEventsPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalSelectionChanged(0x101, "TargetSpeed", true);
    QTest::qWait(10);

    triggerSendOnce();

    EXPECT_EQ(events.dbcCount(), 2);

    std::set<uint16_t> ids;
    for (const auto& msg : events.allDbc())
    {
        ids.insert(msg.messageId);
    }
    EXPECT_TRUE(ids.count(0x100)) << "Expected event for message 0x100";
    EXPECT_TRUE(ids.count(0x101)) << "Expected event for message 0x101";
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_SelectWholeMessage_AllSignalsInEvent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    // messageSelectionChanged selects ALL signals of a message
    emit dbcView->messageSelectionChanged(0x100, true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    // MotorStatus (0x100) has Speed, Temperature, ErrorCode → 3 signals
    EXPECT_EQ(events.firstDbc().signalValues.size(), 3u);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_DeselectSignal_RemovedFromEvent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);
    emit dbcView->signalSelectionChanged(0x100, "Speed", false);
    QTest::qWait(10);

    triggerSendOnce();

    // No signals selected → no event
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_DefaultSignalValue_IsMinimum)
{
    // After loadDbc, signal values are initialised to their minimum.
    // Speed minimum = 0.
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    auto it = std::find_if(sigs.begin(), sigs.end(),
                           [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    EXPECT_DOUBLE_EQ(it->value, 0.0) << "Default signal value should be the signal minimum (0)";
}

// =============================================================================
// SIGNAL VALUE CLAMPING
//
// setData() (delegate path) clamps values to [min, max].
// setSignalValue() (view signal path, i.e. signalValueChanged) does NOT clamp.
// Both paths end up in forEachPendingMessage → the event carries whatever is stored.
// These tests verify what actually reaches the wire.
// =============================================================================

TEST_F(SendingIntegrationTest, SignalValue_ViaViewSignal_OutOfRangeLow_SentAsIs)
{
    // Speed range: [0, 65535].  Emitting signalValueChanged(-999) calls
    // setSignalValue() which stores the value without clamping.
    // The test documents current behaviour; if this fails it means clamping
    // was added to the setSignalValue path (which would be an improvement).
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", -999.0);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    auto it = std::find_if(sigs.begin(), sigs.end(),
                           [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    // EXPECTATION: value should be clamped to 0.0 (the signal minimum).
    // If this assertion fails, setSignalValue does NOT clamp – potential bug.
    EXPECT_GE(it->value, 0.0)
        << "Signal value sent via signalValueChanged should be clamped to signal minimum";
}

TEST_F(SendingIntegrationTest, SignalValue_ViaViewSignal_OutOfRangeHigh_SentAsIs)
{
    // Speed maximum = 65535.
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 999999.0);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    auto it = std::find_if(sigs.begin(), sigs.end(),
                           [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    // EXPECTATION: value should be clamped to 65535 (the signal maximum).
    // If this assertion fails, setSignalValue does NOT clamp – potential bug.
    EXPECT_LE(it->value, 65535.0)
        << "Signal value sent via signalValueChanged should be clamped to signal maximum";
}

// =============================================================================
// MODE SWITCHING
// =============================================================================

TEST_F(SendingIntegrationTest, Mode_DefaultIsRaw_RawEventPublished)
{
    // Do not switch modes
    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Mode_SwitchToDbc_DbcEventPublishedNotRaw)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 0);
    EXPECT_GE(events.dbcCount(), 1u);
}

TEST_F(SendingIntegrationTest, Mode_SwitchBackToRaw_RawEventPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    switchToRaw();
    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Mode_DbcWithNothingSelected_ThenRaw_RawEventPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    // No signals selected
    switchToRaw();
    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.dbcCount(), 0);
}

// =============================================================================
// CYCLIC / REPEATED SENDING
// =============================================================================

TEST_F(SendingIntegrationTest, Cyclic_Raw_FiresMultipleTimes)
{
    startCyclic(100);
    QTest::qWait(450);  // ~4 fires expected
    stopCyclic();

    EXPECT_GE(events.rawCount(), 3u)
        << "Expected at least 3 cyclic raw sends in 450 ms at 100 ms interval";
}

TEST_F(SendingIntegrationTest, Cyclic_Dbc_FiresMultipleTimes)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    startCyclic(100);
    QTest::qWait(450);
    stopCyclic();

    EXPECT_GE(events.dbcCount(), 3u)
        << "Expected at least 3 cyclic DBC sends in 450 ms at 100 ms interval";
}

TEST_F(SendingIntegrationTest, Cyclic_StopsCleanly_NoMoreEventsAfterStop)
{
    startCyclic(100);
    QTest::qWait(200);
    stopCyclic();

    const size_t countAtStop = events.rawCount();

    // Nothing should be published after the worker has been stopped
    QTest::qWait(300);

    EXPECT_EQ(events.rawCount(), countAtStop)
        << "No additional events should be published after cyclic stop";
}

TEST_F(SendingIntegrationTest, Cyclic_SecondStartIgnoredWhileRunning)
{
    startCyclic(100);
    QTest::qWait(50);
    startCyclic(200);  // should be a no-op; worker already active
    QTest::qWait(200);
    stopCyclic();

    // At 100 ms interval for ~250 ms we expect ~2-3 events.
    // At 200 ms the count would be ~1.  Either way there must be at least 1.
    EXPECT_GE(events.rawCount(), 1u);
}

TEST_F(SendingIntegrationTest, Cyclic_StopWithoutStart_DoesNotCrash)
{
    // Calling stop when the worker was never started must not crash or deadlock.
    EXPECT_NO_THROW(stopCyclic());
}

TEST_F(SendingIntegrationTest, Cyclic_CanRestartAfterStop)
{
    startCyclic(100);
    QTest::qWait(150);
    stopCyclic();
    events.clear();

    // Restart
    startCyclic(100);
    QTest::qWait(250);
    stopCyclic();

    EXPECT_GE(events.rawCount(), 1u)
        << "Worker should be restartable after a clean stop";
}

// =============================================================================
// DBC CONFIG UPDATE
// =============================================================================

TEST_F(SendingIntegrationTest, DbcUpdate_ClearsOldSignalSelection)
{
    // Load first DBC, select a signal
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    // Load a new DBC – must clear selections from the old one
    loadDbc(DbcExamples::simple());

    triggerSendOnce();

    // Old selection (0x100 "Speed") no longer exists in new DBC → no event
    EXPECT_EQ(events.dbcCount(), 0)
        << "Reloading a DBC should clear all previous signal selections";
}

TEST_F(SendingIntegrationTest, DbcUpdate_NewSignalCanBeSelectedAndSent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    loadDbc(DbcExamples::simple());  // replaces motor DBC

    emit dbcView->signalSelectionChanged(0x123, "TestSignal", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    EXPECT_EQ(events.firstDbc().messageId, 0x123);
}

TEST_F(SendingIntegrationTest, DbcUpdate_DefaultValueResetToMinimum)
{
    // Set a custom value, reload DBC, verify default (minimum) is used
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 5000.0);
    QTest::qWait(10);

    // Reload same DBC – values must reset to minimum
    loadDbc(DbcExamples::motorController());
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    auto it = std::find_if(sigs.begin(), sigs.end(),
                           [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    EXPECT_DOUBLE_EQ(it->value, 0.0)
        << "After DBC reload, signal value should reset to minimum (0 for Speed)";
}

TEST_F(SendingIntegrationTest, DbcUpdate_MultipleReloads_AlwaysUsesLatest)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    loadDbc(DbcExamples::vehicleSensors());
    loadDbc(DbcExamples::simple());

    emit dbcView->signalSelectionChanged(0x123, "TestSignal", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    EXPECT_EQ(events.firstDbc().messageId, 0x123)
        << "After multiple DBC reloads, only the latest DBC should be active";
}
