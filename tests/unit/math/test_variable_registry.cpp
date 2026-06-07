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

#include <gtest/gtest.h>

#include "math/service/variable_registry.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"
#include "tests/helpers/mock_event_broker.hpp"

class VariableRegistryTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        broker = std::make_unique<TestHelpers::MockEventBroker>();
        registry = std::make_unique<Math::VariableRegistry>(*broker);
    }

    std::unique_ptr<TestHelpers::MockEventBroker> broker;
    std::unique_ptr<Math::VariableRegistry> registry;
};

TEST_F(VariableRegistryTest, AcquireCreatesVariable)
{
    auto* var = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->configKey(), "time:seconds");
}

TEST_F(VariableRegistryTest, AcquireSameKeyReturnsSamePointer)
{
    auto* first = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    auto* second = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    EXPECT_EQ(first, second);
}

TEST_F(VariableRegistryTest, AcquireDifferentKeysReturnsDifferentPointers)
{
    auto* a = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    auto* b = registry->acquire("time:milliseconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Milliseconds);
    });
    EXPECT_NE(a, b);
}

TEST_F(VariableRegistryTest, ReleaseDecrementsRefCount)
{
    registry->acquire("time:seconds",
                      [] { return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds); });
    registry->acquire("time:seconds",
                      [] { return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds); });
    registry->release("time:seconds");
    auto* var = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    EXPECT_NE(var, nullptr);
}

TEST_F(VariableRegistryTest, ReleaseToZeroRemovesVariable)
{
    auto* first = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    registry->release("time:seconds");

    auto* second = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });
    EXPECT_NE(second, nullptr);
}

TEST_F(VariableRegistryTest, ReleaseNonexistentKeyDoesNotCrash)
{
    EXPECT_NO_THROW(registry->release("nonexistent:key"));
}

TEST_F(VariableRegistryTest, UpdateAllCallsUpdateOnEveryVariable)
{
    auto* timeVar = registry->acquire("time:seconds", [] {
        return std::make_unique<Math::TimeVariable>(Math::TimeUnit::Seconds);
    });

    EXPECT_DOUBLE_EQ(*timeVar->ptr(), 0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    registry->updateAll();
    EXPECT_GT(*timeVar->ptr(), 0.0);
}

TEST_F(VariableRegistryTest, DbcConfigNullByDefault)
{
    EXPECT_EQ(registry->dbcConfig(), nullptr);
}

TEST_F(VariableRegistryTest, CanSignalVariableRegisteredInLookupMap)
{
    auto* var = registry->acquire("signal:256:RPM", [] {
        return std::make_unique<Math::CanSignalVariable>(256, "RPM", "Motor");
    });
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->configKey(), "signal:256:RPM");
}

TEST_F(VariableRegistryTest, CanSignalReleaseRemovesFromLookupMap)
{
    registry->acquire("signal:256:RPM", [] {
        return std::make_unique<Math::CanSignalVariable>(256, "RPM", "Motor");
    });
    registry->release("signal:256:RPM");
    auto* var = registry->acquire("signal:256:RPM", [] {
        return std::make_unique<Math::CanSignalVariable>(256, "RPM", "Motor");
    });
    EXPECT_NE(var, nullptr);
}
