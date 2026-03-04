#include "can_handler/can_communication_handler/can_device_handler.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/can_interface_builder.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace CanHandler;

class CanDeviceHandlerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canDeviceHandler = std::make_unique<CanDeviceHandler>(*eventBroker);
    }

    void TearDown() override
    {
        canDeviceHandler.reset();
        eventBroker.reset();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<CanDeviceHandler> canDeviceHandler;
};

TEST_F(CanDeviceHandlerTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(CanDeviceHandler(*eventBroker));
}

TEST_F(CanDeviceHandlerTest, SubscribesToExpectedEvents)
{
    EXPECT_EQ(eventBroker->subscriptionCount<Core::CanDriverChangeEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::GetAvailableCanDriversEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::CheckCanDeviceReadyEvent>(), 1);
}

TEST_F(CanDeviceHandlerTest, CheckCanDeviceReadyReturnsFalseWithoutDriver)
{
    bool ready = true;
    eventBroker->publish(Core::CheckCanDeviceReadyEvent(ready));

    EXPECT_FALSE(ready);
}

TEST_F(CanDeviceHandlerTest, CheckForCanMessageReturnsEmptyWithoutDriver)
{
    EXPECT_TRUE(canDeviceHandler->checkForCanMessage().empty());
}

TEST_F(CanDeviceHandlerTest, SendCanMessageReturnsFalseWithoutDriver)
{
    const CanMessage canMessage{1, {'0', '0', '0', '0', '0', '0', '0', '0'}};

    EXPECT_FALSE(canDeviceHandler->sendCanMessage(canMessage));
}

TEST_F(CanDeviceHandlerTest, InvalidCanDeviceChangeKeepsHandlerNotReady)
{
    EXPECT_NO_THROW(eventBroker->publish(
        Core::CanDriverChangeEvent("definitely_not_a_real_can_interface_123")));

    bool ready = true;
    eventBroker->publish(Core::CheckCanDeviceReadyEvent(ready));

    EXPECT_FALSE(ready);
}

TEST_F(CanDeviceHandlerTest, GetAvailableDevicesEventDoesNotThrow)
{
    std::list<Core::SelectOption> options;

    EXPECT_NO_THROW(eventBroker->publish(Core::GetAvailableCanDriversEvent(&options)));
}

