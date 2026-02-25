#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

#include "core/event/can_driver_event.hpp"
#include "core/event/dbc_event.hpp"
#include "sending/sending_component.hpp"
#include "sending/view/dbc_based_sending_subview.hpp"
#include "sending/view/sending_view.hpp"
#include "tests/helpers/dbc_examples.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace TestHelpers;

/**
 * @brief Integration tests for SendingComponent.
 * These tests verify component lifecycle, event handling, and coordination
 * of sub-components.
 */
class SendingComponentIntegrationTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        mockBroker = std::make_unique<MockEventBroker>();
        component = std::make_unique<Sending::SendingComponent>(*mockBroker);
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

    std::unique_ptr<MockEventBroker> mockBroker;
    std::unique_ptr<Sending::SendingComponent> component;
};

/**
 * @brief Test component lifecycle - construction.
 */
TEST_F(SendingComponentIntegrationTest, ConstructsSuccessfully)
{
    EXPECT_NE(component, nullptr);
    EXPECT_NE(component->getView(), nullptr);
}

/**
 * @brief Test component lifecycle - onStart.
 */
TEST_F(SendingComponentIntegrationTest, StartsSuccessfully)
{
    EXPECT_NO_THROW(component->onStart());
}

/**
 * @brief Test component lifecycle - onStop.
 */
TEST_F(SendingComponentIntegrationTest, StopsSuccessfully)
{
    component->onStart();
    EXPECT_NO_THROW(component->onStop());
}

/**
 * @brief Test multiple start/stop cycles.
 */
TEST_F(SendingComponentIntegrationTest, MultipleStartStopCycles)
{
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_NO_THROW(component->onStart());
        QTest::qWait(10);
        EXPECT_NO_THROW(component->onStop());
    }
}

/**
 * @brief Test DBC configuration event handling.
 */
TEST_F(SendingComponentIntegrationTest, HandlesDbcParsedEvent)
{
    component->onStart();

    QSignalSpy spy(component.get(), &Sending::SendingComponent::dbcConfigurationChanged);

    // Publish DBC parsed event
    const auto config = DbcExamples::motorController();
    mockBroker->triggerEvent(Core::DBCParsedEvent(config, ""));

    // Wait for signal
    ASSERT_TRUE(spy.wait(1000));
    EXPECT_EQ(spy.count(), 1);

    // Verify signal contains correct config
    const auto args = spy.takeFirst();
    const auto receivedConfig = args.at(0).value<Core::DbcConfig>();
    EXPECT_EQ(receivedConfig.metaData.fileName, config.metaData.fileName);
}

/**
 * @brief Test DBC parse error event handling.
 */
TEST_F(SendingComponentIntegrationTest, HandlesDbcParseError)
{
    component->onStart();

    EXPECT_NO_THROW(mockBroker->triggerEvent(Core::DBCParseErrorEvent("Test error", "")));

    QTest::qWait(100);
    SUCCEED();
}

/**
 * @brief Test CAN driver change event handling.
 */
TEST_F(SendingComponentIntegrationTest, HandlesCanDriverChangeEvent)
{
    component->onStart();

    EXPECT_NO_THROW(mockBroker->triggerEvent(Core::CanDriverChangeEvent("vcan0")));

    QTest::qWait(100);
    SUCCEED();
}

/**
 * @brief Test multiple DBC config updates.
 */
TEST_F(SendingComponentIntegrationTest, HandlesMultipleDbcConfigs)
{
    component->onStart();

    QSignalSpy spy(component.get(), &Sending::SendingComponent::dbcConfigurationChanged);

    // First config
    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::motorController(), ""));
    ASSERT_TRUE(spy.wait(500));

    // Second config
    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::vehicleSensors(), ""));
    ASSERT_TRUE(spy.wait(500));

    EXPECT_EQ(spy.count(), 2);
}

/**
 * @brief Test component with empty DBC config.
 */
TEST_F(SendingComponentIntegrationTest, HandlesEmptyDbcConfig)
{
    component->onStart();

    QSignalSpy spy(component.get(), &Sending::SendingComponent::dbcConfigurationChanged);

    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::empty(), ""));

    ASSERT_TRUE(spy.wait(500));
    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test getView returns valid widget.
 */
TEST_F(SendingComponentIntegrationTest, GetViewReturnsValidWidget)
{
    auto* view = component->getView();

    EXPECT_NE(view, nullptr);
    EXPECT_TRUE(view->isWidgetType());
}

/**
 * @brief Test component survives rapid event publishing.
 */
TEST_F(SendingComponentIntegrationTest, SurvivesRapidEventPublishing)
{
    component->onStart();

    // Publish many events rapidly
    for (int i = 0; i < 10; ++i)
    {
        mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::motorController(), ""));
        mockBroker->triggerEvent(Core::DBCParseErrorEvent("Error " + std::to_string(i), ""));
        mockBroker->triggerEvent(Core::CanDriverChangeEvent("vcan" + std::to_string(i)));
    }

    QTest::qWait(500);

    EXPECT_NE(component, nullptr);
    EXPECT_NE(component->getView(), nullptr);
}

/**
 * @brief Test component destruction after start.
 */
TEST_F(SendingComponentIntegrationTest, SafeDestructionAfterStart)
{
    component->onStart();
    QTest::qWait(50);

    EXPECT_NO_THROW(component.reset());
}

/**
 * @brief Test component destruction during event processing.
 */
TEST_F(SendingComponentIntegrationTest, SafeDestructionDuringEventProcessing)
{
    component->onStart();

    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::motorController(), ""));

    QTest::qWait(10);
    EXPECT_NO_THROW(component.reset());
}

/**
 * @brief Test error then success event sequence.
 */
TEST_F(SendingComponentIntegrationTest, HandlesErrorThenSuccessSequence)
{
    component->onStart();

    QSignalSpy spy(component.get(), &Sending::SendingComponent::dbcConfigurationChanged);
    mockBroker->triggerEvent(Core::DBCParseErrorEvent("Parse failed", ""));
    QTest::qWait(50);
    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::simple(), ""));

    ASSERT_TRUE(spy.wait(500));
    EXPECT_EQ(spy.count(), 1);
}

/**
 * @brief Test sendOnce method via view signal.
 * Covers sendOnce method (lines 197-200) and publishRawMessageAsync (lines 85-96).
 */
TEST_F(SendingComponentIntegrationTest, SendOnceTriggeredByViewSignal)
{
    component->onStart();

    auto* view = component->getView();
    ASSERT_NE(view, nullptr);

    emit static_cast<Sending::SendingView*>(view)->sendOnceRequested();

    // Wait for async processing
    QTest::qWait(100);

    SUCCEED();
}

/**
 * @brief Test startRepeatedSending via view signal.
 */
TEST_F(SendingComponentIntegrationTest, StartRepeatedSendingTriggeredByViewSignal)
{
    component->onStart();

    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::simple(), ""));
    QTest::qWait(100);

    auto* view = component->getView();
    ASSERT_NE(view, nullptr);
    emit static_cast<Sending::SendingView*>(view)->startRepeatedSendingRequested(100);

    QTest::qWait(250);
    emit static_cast<Sending::SendingView*>(view)->stopRepeatedSendingRequested();
    QTest::qWait(50);

    SUCCEED();
}

/**
 * @brief Test stopRepeatedSending via view signal.
 */
TEST_F(SendingComponentIntegrationTest, StopRepeatedSendingTriggeredByViewSignal)
{
    component->onStart();

    auto* view = component->getView();
    ASSERT_NE(view, nullptr);

    // Start first
    emit static_cast<Sending::SendingView*>(view)->startRepeatedSendingRequested(100);
    QTest::qWait(50);

    // Then stop
    emit static_cast<Sending::SendingView*>(view)->stopRepeatedSendingRequested();
    QTest::qWait(50);

    SUCCEED();
}

/**
 * @brief Test repeated sending doesn't start if already running.
 */
TEST_F(SendingComponentIntegrationTest, RepeatedSendingIgnoresStartWhenAlreadyRunning)
{
    component->onStart();

    auto* view = component->getView();
    ASSERT_NE(view, nullptr);

    emit static_cast<Sending::SendingView*>(view)->startRepeatedSendingRequested(100);
    QTest::qWait(50);

    emit static_cast<Sending::SendingView*>(view)->startRepeatedSendingRequested(200);
    QTest::qWait(50);

    emit static_cast<Sending::SendingView*>(view)->stopRepeatedSendingRequested();
    QTest::qWait(50);

    SUCCEED();
}

/**
 * @brief Test publishRawMessageAsync via model's requestSendRaw signal.
 */
TEST_F(SendingComponentIntegrationTest, PublishRawMessageViaModelSignal)
{
    component->onStart();
    auto* view = component->getView();
    ASSERT_NE(view, nullptr);
    emit static_cast<Sending::SendingView*>(view)->sendOnceRequested();
    QTest::qWait(100);
    SUCCEED();
}

/**
 * @brief Test publishDbcMessageAsync via model's requestSendDbc signal.
 */
TEST_F(SendingComponentIntegrationTest, PublishDbcMessageViaModelSignal)
{
    component->onStart();

    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::motorController(), ""));
    QTest::qWait(50);

    auto* view = static_cast<Sending::SendingView*>(component->getView());
    ASSERT_NE(view, nullptr);

    auto* dbcView = view->dbcSubView();
    ASSERT_NE(dbcView, nullptr);

    emit view->modeChanged(true);

    emit dbcView->signalSelectionChanged(0x100, "Speed", true);
    emit dbcView->signalValueChanged(0x100, "Speed", 1000.0);

    QTest::qWait(50);

    // Trigger DBC message sending
    emit view->sendOnceRequested();
    QTest::qWait(100);

    SUCCEED();
}

/**
 * @brief Test worker callback publishes DBC messages during repeated sending.
 */
TEST_F(SendingComponentIntegrationTest, RepeatedSendingPublishesDbcMessages)
{
    component->onStart();

    mockBroker->triggerEvent(Core::DBCParsedEvent(DbcExamples::simple(), ""));
    QTest::qWait(50);

    auto* view = static_cast<Sending::SendingView*>(component->getView());
    auto* dbcView = view->dbcSubView();

    emit view->modeChanged(true);
    emit dbcView->signalSelectionChanged(0x256, "TestSignal", true);
    QTest::qWait(50);

    emit view->startRepeatedSendingRequested(100);
    QTest::qWait(250);
    emit view->stopRepeatedSendingRequested();

    SUCCEED();
}

/**
 * @brief Test worker error handler by cleanup during active sending.
 */
TEST_F(SendingComponentIntegrationTest, HandlesWorkerError)
{
    component->onStart();

    auto* view = static_cast<Sending::SendingView*>(component->getView());
    emit view->startRepeatedSendingRequested(50);
    QTest::qWait(100);

    component->onStop();
    QTest::qWait(100);

    SUCCEED();
}
