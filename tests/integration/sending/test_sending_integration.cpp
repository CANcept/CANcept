#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QTest>
#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include "core/event/can_event.hpp"
#include "core/event/dbc_event.hpp"
#include "core/widgets/common/styled_switch.hpp"
#include "sending/sending_component.hpp"
#include "sending/view/dbc_based_sending_subview.hpp"
#include "sending/view/raw_sending_subview.hpp"
#include "sending/view/sending_view.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace TestHelpers;
using ::testing::_;
using ::testing::AnyNumber;

struct CapturedSendEvents {
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
        broker = std::make_shared<MockEventBroker>();

        // calls during construction
        EXPECT_CALL(*broker, _subscribeEvent(_)).Times(AnyNumber());
        EXPECT_CALL(*broker, _publishEvent(_, _)).Times(AnyNumber());

        // Capture send events
        m_rawConn = broker->subscribe<Core::SendCanMessageRawEvent>(
            [this](const Core::SendCanMessageRawEvent& e) {
                std::lock_guard lock(events.mutex);
                events.raw.push_back(e);
            });
        m_dbcConn = broker->subscribe<Core::SendCanMessageDbcEvent>(
            [this](const Core::SendCanMessageDbcEvent& e) {
                std::lock_guard lock(events.mutex);
                events.dbc.push_back(e);
            });

        component = std::make_unique<Sending::SendingComponent>(*broker);
        component->onStart();

        view = static_cast<Sending::SendingView*>(component->getView());
        rawView = view->rawSubView();
        dbcView = view->dbcSubView();

        // Let onStart() settle
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
        m_rawConn = {};
        m_dbcConn = {};
        broker.reset();
    }

    /** Trigger a DBCParsedEvent into the component and wait for the queued UI dispatch. */
    void loadDbc(const Core::DbcConfig& config) const
    {
        broker->triggerEvent(Core::DBCParsedEvent(config, "test.dbc"));
        QTest::qWait(150);
    }

    /** Switch the model to DBC mode (simulating the sidebar). */
    void switchToDbc() const
    {
        emit view->modeChanged(true);
        QTest::qWait(10);
    }

    /** Switch the model back to Raw mode. */
    void switchToRaw() const
    {
        emit view->modeChanged(false);
        QTest::qWait(10);
    }

    /**
     * Trigger a single send and wait for the QtConcurrent::run thread to
     * publish to the broker.
     */
    void triggerSendOnce() const
    {
        emit view->sendOnceRequested();
        QTest::qWait(200);
    }

    void startCyclic(const int intervalMs) const
    {
        emit view->startRepeatedSendingRequested(intervalMs);
    }

    void stopCyclic() const
    {
        emit view->stopRepeatedSendingRequested();
        QTest::qWait(300);  // wait for worker
    }

    std::shared_ptr<MockEventBroker> broker;
    std::unique_ptr<Sending::SendingComponent> component;
    Sending::SendingView* view = nullptr;
    Sending::RawSendingSubView* rawView = nullptr;
    Sending::DbcSendingSubView* dbcView = nullptr;
    CapturedSendEvents events;
    Core::Connection m_rawConn;
    Core::Connection m_dbcConn;
};

/** Raw Sending */

TEST_F(SendingIntegrationTest, Raw_SendOnce_PublishesExactlyOneRawEvent)
{
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

/** DBC Based Sending */

TEST_F(SendingIntegrationTest, Dbc_SendOnce_NoDbc_NoEventPublished)
{
    switchToDbc();
    triggerSendOnce();

    EXPECT_EQ(events.rawCount(), 0);
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_DbcLoaded_NoSignalsSelected_NoEventPublished)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();

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
    const auto msg = events.firstDbc();
    const auto& sigs = msg.signalValues;
    const bool found = std::ranges::any_of(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    EXPECT_TRUE(found) << "Selected signal 'Speed' should appear in the published event";
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_UnselectedSignalAbsentFromPayload)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    const bool tempFound =
        std::ranges::any_of(sigs.begin(), sigs.end(),
                            [](const Core::DbcCanSignal& s) { return s.name == "Temperature"; });
    const bool errFound =
        std::ranges::any_of(sigs.begin(), sigs.end(),
                            [](const Core::DbcCanSignal& s) { return s.name == "ErrorCode"; });

    EXPECT_FALSE(tempFound) << "Unselected signal 'Temperature' must NOT appear in the event";
    EXPECT_FALSE(errFound) << "Unselected signal 'ErrorCode' must NOT appear in the event";
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
    const auto it = std::ranges::find_if(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
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
    const bool speedFound = std::ranges::any_of(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    const bool tempFound =
        std::ranges::any_of(sigs.begin(), sigs.end(),
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
    EXPECT_EQ(events.dbcCount(), 0);
}

TEST_F(SendingIntegrationTest, Dbc_SendOnce_DefaultSignalValue_IsMinimum)
{
    // After loadDbc, signal values are initialised to their minimum.
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    const auto it = std::ranges::find_if(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    EXPECT_DOUBLE_EQ(it->value, 0.0) << "Default signal value should be the signal minimum (0)";
}

/** Signal Value Clamping */

TEST_F(SendingIntegrationTest, SignalValue_ViaViewSignal_OutOfRangeLow_SentAsIs)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", -999.0);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    const auto it = std::ranges::find_if(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    // should clamp to signal minimum
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
    const auto it = std::ranges::find_if(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, sigs.end());
    // should clamp to signal maximum
    EXPECT_LE(it->value, 65535.0)
        << "Signal value sent via signalValueChanged should be clamped to signal maximum";
}

/** Mode */

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

/** Repeated Sending */

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
    startCyclic(200);
    QTest::qWait(200);
    stopCyclic();

    // atleast 1
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

    EXPECT_GE(events.rawCount(), 1u) << "Worker should be restartable after a clean stop";
}

/** DBC load */
TEST_F(SendingIntegrationTest, DbcUpdate_ClearsOldSignalSelection)
{
    // Load first DBC
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    // Load a new DBC
    loadDbc(DbcExamples::simple());

    triggerSendOnce();

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
    const auto it = std::ranges::find_if(
        sigs.begin(), sigs.end(), [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
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

// =============================================================================
// COMPLETE FLOW TESTS — button clicks, state machines, interaction chains
// =============================================================================

// Clicking the send button directly (not emitting sendOnceRequested) exercises the
// real connection chain: button::clicked → view slot → sendOnceRequested → component.
TEST_F(SendingIntegrationTest, Flow_Raw_TypeInputs_ClickSendButton_EventMatchesInput)
{
    rawView->canIdEditor()->setText("3BC");
    rawView->messageDataEditor()->setText("11 22 33 44");
    QTest::qWait(20);

    rawView->sendButton()->click();
    QTest::qWait(200);

    ASSERT_EQ(events.rawCount(), 1);
    const auto msg = events.firstRaw();
    EXPECT_EQ(msg.messageId, 0x3BC);
    EXPECT_EQ(msg.dlc, 4);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[0]), 0x11);
    EXPECT_EQ(static_cast<uint8_t>(msg.data[3]), 0x44);
}

// Enable the cyclic toggle → set interval → click send button to START →
// verify button transitions to "Stop" state → click again to STOP.
// Tests the full send-button state machine through real widget interactions.
TEST_F(SendingIntegrationTest, Flow_Raw_CyclicStateMachine_ToggleEnableClickStartClickStop)
{
    rawView->repeatedSendingCard()->findChild<Core::StyledSwitch*>()->setChecked(true);
    rawView->repeatedSendingCard()->frequencyEditor()->setText("100");
    QTest::qWait(20);

    // Click send — cyclic is enabled, so this starts the worker
    rawView->sendButton()->click();
    QTest::qWait(50);  // let isSending propagate to button

    EXPECT_EQ(rawView->sendButton()->text(), "Stop")
        << "Button should show 'Stop' once repeated sending is active";

    QTest::qWait(350);  // accumulate sends

    // Click the Stop button
    rawView->sendButton()->click();
    QTest::qWait(300);

    EXPECT_NE(rawView->sendButton()->text(), "Stop") << "Button should revert after stopping";
    EXPECT_GE(events.rawCount(), 2u)
        << "Multiple events should have been published during cyclic sending";
}

// DBC send button must be disabled when no signals are selected, and re-enabled
// as soon as at least one signal is selected.
TEST_F(SendingIntegrationTest, Flow_Dbc_SendButton_DisabledWithNoSelection_EnabledAfterSelect)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();

    EXPECT_FALSE(dbcView->sendButton()->isEnabled())
        << "DBC send button must be disabled when no signals are selected";

    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    EXPECT_TRUE(dbcView->sendButton()->isEnabled())
        << "DBC send button must be enabled once at least one signal is selected";
}

// Deselecting the last signal must disable the send button again.
TEST_F(SendingIntegrationTest, Flow_Dbc_SendButton_DisabledAgainAfterDeselectingLastSignal)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);
    ASSERT_TRUE(dbcView->sendButton()->isEnabled());

    emit dbcView->signalSelectionChanged(0x100, "Speed", false);
    QTest::qWait(10);

    EXPECT_FALSE(dbcView->sendButton()->isEnabled())
        << "Send button must disable once the last signal is deselected";
}

// Full DBC flow via actual button click: select signal, set value, click the DBC send button.
TEST_F(SendingIntegrationTest, Flow_Dbc_ClickSendButton_PublishesCorrectEvent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x101, "TargetSpeed", true);
    emit dbcView->signalValueChanged(0x101, "TargetSpeed", 4200.0);
    QTest::qWait(10);

    dbcView->sendButton()->click();
    QTest::qWait(200);

    ASSERT_EQ(events.dbcCount(), 1);
    EXPECT_EQ(events.firstDbc().messageId, 0x101);
    const auto& sigs = events.firstDbc().signalValues;
    const auto it = std::ranges::find_if(
        sigs, [](const Core::DbcCanSignal& s) { return s.name == "TargetSpeed"; });
    ASSERT_NE(it, sigs.end());
    EXPECT_DOUBLE_EQ(it->value, 4200.0);
}

// Selecting/deselecting/reselecting a signal and then sending must only include
// signals that are currently selected at send time.
TEST_F(SendingIntegrationTest, Flow_Dbc_SelectDeselectReselect_OnlyCurrentSelectionSent)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalSelectionChanged(0x100, "Temperature", true);
    QTest::qWait(10);

    // Deselect Temperature — only Speed should go out
    emit dbcView->signalSelectionChanged(0x100, "Temperature", false);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    EXPECT_EQ(sigs.size(), 1u);
    EXPECT_EQ(sigs.front().name, "Speed");
}

// Message selection checkbox selects all signals; sending must include them all.
// Message deselection must then exclude all of them.
TEST_F(SendingIntegrationTest, Flow_Dbc_MessageCheckbox_SelectThenDeselect_FullRoundTrip)
{
    loadDbc(DbcExamples::motorController());  // 0x100 has Speed, Temperature, ErrorCode
    switchToDbc();

    emit dbcView->messageSelectionChanged(0x100, true);
    QTest::qWait(10);
    triggerSendOnce();
    ASSERT_EQ(events.dbcCount(), 1);
    EXPECT_EQ(events.firstDbc().signalValues.size(), 3u) << "All 3 signals selected via message";

    events.clear();
    emit dbcView->messageSelectionChanged(0x100, false);
    QTest::qWait(10);
    triggerSendOnce();
    EXPECT_EQ(events.dbcCount(), 0) << "No signals after message deselect";
}

// Switching mode while a cyclic worker is active: the worker keeps running but
// subsequent sends use the new mode's data.  Specifically, switching to DBC with
// no signals selected should produce zero DBC events for that period.
TEST_F(SendingIntegrationTest, Flow_ModeSwitchDuringActiveCyclic_SwitchesToNewModeData)
{
    // Start raw cyclic
    startCyclic(100);
    QTest::qWait(250);
    const size_t rawBefore = events.rawCount();
    EXPECT_GE(rawBefore, 1u);

    // Switch to DBC with no signals selected — cyclic keeps ticking but
    // DBC mode with no selection produces no events
    switchToDbc();
    events.clear();
    QTest::qWait(300);
    stopCyclic();

    // With no DBC signals selected, no events of either type should arrive
    EXPECT_EQ(events.dbcCount(), 0u)
        << "DBC mode with no signals selected should publish no events";
    EXPECT_EQ(events.rawCount(), 0u) << "Mode switched to DBC, no raw events expected";
}

// Load DBC, start cyclic, reload a different DBC mid-flight.
// The selection is cleared on reload so no events should be produced after the reload.
TEST_F(SendingIntegrationTest, Flow_DbcReloadDuringActiveCyclic_OldSelectionCleared)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    startCyclic(80);
    QTest::qWait(200);
    EXPECT_GE(events.dbcCount(), 1u) << "Should have sends before reload";

    events.clear();
    loadDbc(DbcExamples::simple());  // clears selection — 0x100/Speed no longer exists
    QTest::qWait(250);
    stopCyclic();

    EXPECT_EQ(events.dbcCount(), 0u)
        << "After DBC reload, old selection is cleared; no events expected";
}

// All three messages from vehicleSensors selected; cyclic send must publish
// events for each message ID in every cycle.
TEST_F(SendingIntegrationTest, Flow_Dbc_AllMessagesSelected_CyclicPublishesAllMessageIds)
{
    loadDbc(DbcExamples::vehicleSensors());  // 0x200, 0x201, 0x202
    switchToDbc();
    emit dbcView->messageSelectionChanged(0x200, true);
    emit dbcView->messageSelectionChanged(0x201, true);
    emit dbcView->messageSelectionChanged(0x202, true);
    QTest::qWait(10);

    startCyclic(100);
    QTest::qWait(450);
    stopCyclic();

    const auto msgs = events.allDbc();
    std::set<uint16_t> seenIds;
    for (const auto& m : msgs)
    {
        seenIds.insert(m.messageId);
    }
    EXPECT_TRUE(seenIds.count(0x200)) << "0x200 VehicleSpeed must appear";
    EXPECT_TRUE(seenIds.count(0x201)) << "0x201 Temperatures must appear";
    EXPECT_TRUE(seenIds.count(0x202)) << "0x202 BatteryStatus must appear";
}

// Change a signal value while cyclic is running; events collected after the
// change must carry the new value.
TEST_F(SendingIntegrationTest, Flow_Dbc_ChangeSignalValueDuringCyclic_NewValueReachesWire)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 1000.0);
    QTest::qWait(10);

    startCyclic(80);
    QTest::qWait(200);
    events.clear();  // discard events with old value

    emit dbcView->signalValueChanged(0x100, "Speed", 9999.0);
    QTest::qWait(300);
    stopCyclic();

    ASSERT_FALSE(events.allDbc().empty());
    const auto& last = events.allDbc().back();
    const auto it = std::ranges::find_if(
        last.signalValues, [](const Core::DbcCanSignal& s) { return s.name == "Speed"; });
    ASSERT_NE(it, last.signalValues.end());
    EXPECT_DOUBLE_EQ(it->value, 9999.0) << "Events after value change must carry the updated value";
}

// After stopping cyclic and restarting it, new events must accumulate.
// Tests that the worker can be recycled cleanly multiple times.
TEST_F(SendingIntegrationTest, Flow_Cyclic_MultipleStartStopCycles_AllProduceEvents)
{
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        events.clear();
        startCyclic(100);
        QTest::qWait(300);
        stopCyclic();

        EXPECT_GE(events.rawCount(), 1u) << "Cycle " << cycle << ": expected at least one event";
    }
}

// With DBC cyclic enabled, select signals from TWO messages; each cyclic tick
// must publish one event per message.  After 300 ms at 100 ms interval we
// expect at least 3 ticks × 2 messages = 6 events.
TEST_F(SendingIntegrationTest, Flow_Dbc_TwoMessages_CyclicRateMatchesExpectedEventCount)
{
    loadDbc(DbcExamples::motorController());  // 0x100 and 0x101
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalSelectionChanged(0x101, "TargetSpeed", true);
    QTest::qWait(10);

    startCyclic(100);
    QTest::qWait(450);
    stopCyclic();

    EXPECT_GE(events.dbcCount(), 6u)
        << "2 messages × ~3 ticks should produce at least 6 DBC events";
}

// Sending in raw mode with an oversized data string must not exceed 8 bytes DLC.
TEST_F(SendingIntegrationTest, Flow_Raw_MoreThan8DataBytes_DlcCappedAt8)
{
    rawView->messageDataEditor()->setText("01 02 03 04 05 06 07 08 09 0A");
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().dlc, 8) << "DLC must never exceed the CAN maximum of 8";
}

// Entering a CAN ID above 0x7FF must be clamped to the 11-bit standard CAN range.
TEST_F(SendingIntegrationTest, Flow_Raw_CanIdAbove7FF_ClampedToStandardRange)
{
    rawView->canIdEditor()->setText("FFF");  // 4095, above 11-bit max
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_LE(events.firstRaw().messageId, 0x7FFu)
        << "CAN ID must be clamped to the 11-bit standard frame range";
}

// Setting all three signal values on a message and sending must include all three
// with their exact (clamped) values in one event.
TEST_F(SendingIntegrationTest, Flow_Dbc_ThreeSignals_ValuesAllPresentAndCorrect)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalSelectionChanged(0x100, "Temperature", true);
    emit dbcView->signalSelectionChanged(0x100, "ErrorCode", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 2500.0);
    emit dbcView->signalValueChanged(0x100, "Temperature", 85.0);  // within [-40, 215]
    emit dbcView->signalValueChanged(0x100, "ErrorCode", 2.0);
    QTest::qWait(10);

    triggerSendOnce();

    ASSERT_EQ(events.dbcCount(), 1);
    const auto& sigs = events.firstDbc().signalValues;
    ASSERT_EQ(sigs.size(), 3u);

    auto find = [&](const std::string& name) {
        return std::ranges::find_if(sigs,
                                    [&](const Core::DbcCanSignal& s) { return s.name == name; });
    };
    ASSERT_NE(find("Speed"), sigs.end());
    EXPECT_DOUBLE_EQ(find("Speed")->value, 2500.0);
    ASSERT_NE(find("Temperature"), sigs.end());
    EXPECT_DOUBLE_EQ(find("Temperature")->value, 85.0);
    ASSERT_NE(find("ErrorCode"), sigs.end());
    EXPECT_DOUBLE_EQ(find("ErrorCode")->value, 2.0);
}

// Switching from DBC back to raw while DBC signals are selected must immediately
// revert to raw event publication.
TEST_F(SendingIntegrationTest, Flow_Dbc_SwitchBackToRaw_RawEventOverridesDbcSelection)
{
    loadDbc(DbcExamples::motorController());
    switchToDbc();
    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    QTest::qWait(10);

    switchToRaw();
    rawView->canIdEditor()->setText("AAA");
    QTest::qWait(10);

    triggerSendOnce();

    EXPECT_EQ(events.dbcCount(), 0) << "Raw mode must not publish DBC events";
    ASSERT_EQ(events.rawCount(), 1);
    EXPECT_EQ(events.firstRaw().messageId, 0xAAA & 0x7FF);
}

// Toggling cyclic off while the worker is running must not crash and must stop
// the worker when the stop button is subsequently clicked.
TEST_F(SendingIntegrationTest, Flow_Cyclic_ToggleOffWhileRunning_StopButtonEndsGracefully)
{
    rawView->repeatedSendingCard()->findChild<Core::StyledSwitch*>()->setChecked(true);
    rawView->repeatedSendingCard()->frequencyEditor()->setText("100");
    QTest::qWait(10);
    rawView->sendButton()->click();  // starts
    QTest::qWait(150);

    // Toggle repeated sending off — the model records this but the worker keeps
    // running until explicitly stopped via the Stop button
    emit rawView->repeatedSendingCard()->toggled(false);
    QTest::qWait(10);

    rawView->sendButton()->click();  // Stop button click
    QTest::qWait(300);

    // No crash and button reverts
    EXPECT_NE(rawView->sendButton()->text(), "Stop");
}

// Sending with an empty DBC (no messages/signals) in DBC mode must publish nothing.
TEST_F(SendingIntegrationTest, Flow_Dbc_EmptyDbc_SendProducesNoEvents)
{
    loadDbc(DbcExamples::empty());
    switchToDbc();

    triggerSendOnce();

    EXPECT_EQ(events.dbcCount(), 0);
    EXPECT_EQ(events.rawCount(), 0);
}

// Rapid successive single-sends must each produce exactly one event and must
// not merge or drop any.
TEST_F(SendingIntegrationTest, Flow_Raw_RapidSuccessiveSends_EachProducesOneEvent)
{
    constexpr int kSends = 5;
    for (int i = 0; i < kSends; ++i)
    {
        emit view->sendOnceRequested();
    }
    QTest::qWait(500);

    EXPECT_EQ(events.rawCount(), static_cast<size_t>(kSends))
        << kSends << " successive sends must produce exactly " << kSends << " events";
}
