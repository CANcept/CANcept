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

#include <thread>

#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"

// CanSignalVariable
TEST(CanSignalVariableTest, SetValueUpdatesPointer)
{
    Math::CanSignalVariable var(0x100, "RPM", "Motor");
    var.setValue(3500.0);
    EXPECT_DOUBLE_EQ(*var.ptr(), 3500.0);
}

TEST(CanSignalVariableTest, SharedPtrKeepsValueAlive)
{
    const auto shared = [&] {
        Math::CanSignalVariable var(0x100, "RPM", "Motor");
        var.setValue(42.0);
        return var.sharedPtr();
    }();
    EXPECT_DOUBLE_EQ(*shared, 42.0);
}

TEST(CanSignalVariableTest, Accessors)
{
    const Math::CanSignalVariable var(0x300, "Temp", "Sensors");
    EXPECT_EQ(var.messageId(), 0x300u);
    EXPECT_EQ(var.signalName(), "Temp");
}

// TimeVariable
TEST(TimeVariableTest, ConfigKeys)
{
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Seconds).configKey(), "time:seconds");
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Milliseconds).configKey(), "time:milliseconds");
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Nanoseconds).configKey(), "time:nanoseconds");
}

TEST(TimeVariableTest, DisplayNames)
{
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Seconds).displayName(), "Time (s)");
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Milliseconds).displayName(), "Time (ms)");
    EXPECT_EQ(Math::TimeVariable(Math::TimeUnit::Nanoseconds).displayName(), "Time (ns)");
}

TEST(TimeVariableTest, InitialValueIsZeroBeforeUpdate)
{
    Math::TimeVariable var(Math::TimeUnit::Seconds);
    EXPECT_DOUBLE_EQ(*var.ptr(), 0.0);
}

TEST(TimeVariableTest, UpdateProducesPositiveValue)
{
    Math::TimeVariable var(Math::TimeUnit::Nanoseconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    var.update();
    EXPECT_GT(*var.ptr(), 0.0);
}

TEST(TimeVariableTest, UpdateIncreasesOverTime)
{
    Math::TimeVariable var(Math::TimeUnit::Milliseconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    var.update();
    const double first = *var.ptr();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    var.update();
    const double second = *var.ptr();

    EXPECT_GT(second, first);
}

TEST(TimeVariableTest, UnitSuffixRoundTrip)
{
    EXPECT_EQ(Math::TimeVariable::unitFromSuffix("seconds"), Math::TimeUnit::Seconds);
    EXPECT_EQ(Math::TimeVariable::unitFromSuffix("milliseconds"), Math::TimeUnit::Milliseconds);
    EXPECT_EQ(Math::TimeVariable::unitFromSuffix("nanoseconds"), Math::TimeUnit::Nanoseconds);
    EXPECT_EQ(Math::TimeVariable::unitFromSuffix("unknown"), Math::TimeUnit::Seconds);
}

TEST(TimeVariableTest, UnitSuffixValues)
{
    EXPECT_EQ(Math::TimeVariable::unitSuffix(Math::TimeUnit::Seconds), "seconds");
    EXPECT_EQ(Math::TimeVariable::unitSuffix(Math::TimeUnit::Milliseconds), "milliseconds");
    EXPECT_EQ(Math::TimeVariable::unitSuffix(Math::TimeUnit::Nanoseconds), "nanoseconds");
}
