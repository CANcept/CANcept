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

#include <thread>

#include "can_handler/can_communication_handler/can_communication_handler.hpp"
#include "can_handler/constants.hpp"
#include "gtest/gtest.h"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/mock_settings_registry.hpp"

using namespace CanHandler;

class CanCommunicationHandlerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        eventBroker = std::make_unique<TestHelpers::MockEventBroker>();
        canCommunicationHandler = std::make_unique<CanCommunicationHandler>(*eventBroker);
    }

    void TearDown() override
    {
        if (isStarted)
        {
            eventBroker->publish(Core::AppStoppedEvent{});
            isStarted = false;
        }

        canCommunicationHandler.reset();
        eventBroker.reset();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> eventBroker;
    std::unique_ptr<CanCommunicationHandler> canCommunicationHandler;
    bool isStarted = false;
};

TEST_F(CanCommunicationHandlerTest, ConstructsSuccessfully)
{
    EXPECT_NO_THROW(CanCommunicationHandler(*eventBroker));
}

TEST_F(CanCommunicationHandlerTest, SubscribesToLifecycleAndCanEvents)
{
    EXPECT_EQ(eventBroker->subscriptionCount<Core::AppStartedEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::AppStoppedEvent>(), 1);

    EXPECT_EQ(eventBroker->subscriptionCount<Core::SendCanMessageRawEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::SendCanMessageDbcEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::DBCParsedEvent>(), 1);

    EXPECT_EQ(eventBroker->subscriptionCount<Core::CanDriverChangeEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::GetAvailableCanDriversEvent>(), 1);
    EXPECT_EQ(eventBroker->subscriptionCount<Core::CheckCanDeviceReadyEvent>(), 1);
}

TEST_F(CanCommunicationHandlerTest, StartAndStopLifecycleDoesNotThrow)
{
    EXPECT_NO_THROW(eventBroker->publish(Core::AppStartedEvent{}));
    isStarted = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_NO_THROW(eventBroker->publish(Core::AppStoppedEvent{}));
    isStarted = false;
}

TEST_F(CanCommunicationHandlerTest, SendRawMessageEventWithoutDeviceDoesNotThrow)
{
    constexpr Core::RawCanMessage rawMessage = {
        std::chrono::milliseconds(1000), {'1', '2', '3', '4', '5', '6', '7', '8'}, 1, 8};

    EXPECT_NO_THROW(eventBroker->publish(Core::SendCanMessageRawEvent(rawMessage)));
}

TEST_F(CanCommunicationHandlerTest, CheckCanDeviceReadyReturnsFalseWithoutConfiguredDriver)
{
    bool ready = true;

    eventBroker->publish(Core::CheckCanDeviceReadyEvent(ready));

    EXPECT_FALSE(ready);
}

TEST_F(CanCommunicationHandlerTest, RegisterSettingsDelegatesToDeviceHandler)
{
    TestHelpers::MockSettingsRegistry registry;
    const std::unique_ptr<Core::ILifecycle> handler =
        std::make_unique<CanCommunicationHandler>(*eventBroker);
    handler->registerSettings(registry);

    ASSERT_EQ(registry.getSettings().size(), 1);
    const auto& setting = registry.getSettings().front();
    EXPECT_EQ(setting->getType(), Core::SettingType::Select);
    EXPECT_EQ(setting->getKey().componentId, Constants::MODULE_ID);
    EXPECT_EQ(setting->getKey().settingId, Constants::DEVICE_SELECTION_SETTING_ID);
}

TEST_F(CanCommunicationHandlerTest, RegisterSettingsDoesNotThrow)
{
    TestHelpers::MockSettingsRegistry registry;

    const std::unique_ptr<Core::ILifecycle> handler =
        std::make_unique<CanCommunicationHandler>(*eventBroker);

    EXPECT_NO_THROW(handler->registerSettings(registry));
}