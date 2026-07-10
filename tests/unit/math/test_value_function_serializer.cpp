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

#include <filesystem>
#include <memory>

#include "gtest/gtest.h"
#include "math/service/value_function_serializer.hpp"
#include "math/service/variable_registry.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"
#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace Math;
using namespace TestHelpers;

namespace {
auto tempFilePath() -> std::filesystem::path
{
    return std::filesystem::temp_directory_path() /
           std::filesystem::path("canbusmanager_value_function_serializer_test_" +
                                 std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
                                 ".json");
}
}  // namespace

TEST(ValueFunctionSerializerTest, MakeVariableFromConfigKeyBuildsTimeVariable)
{
    const auto variable = makeVariableFromConfigKey("time:milliseconds", nullptr);
    ASSERT_NE(variable, nullptr);

    const auto* timeVariable = dynamic_cast<const TimeVariable*>(variable.get());
    ASSERT_NE(timeVariable, nullptr);
    EXPECT_EQ(timeVariable->unit(), TimeUnit::Milliseconds);
}

TEST(ValueFunctionSerializerTest, MakeVariableFromConfigKeyBuildsSignalVariable)
{
    const auto variable = makeVariableFromConfigKey("signal:256:EngineRPM", nullptr);
    ASSERT_NE(variable, nullptr);

    const auto* signalVariable = dynamic_cast<const CanSignalVariable*>(variable.get());
    ASSERT_NE(signalVariable, nullptr);
    EXPECT_EQ(signalVariable->messageId(), 256U);
    EXPECT_EQ(signalVariable->signalName(), "EngineRPM");
}

TEST(ValueFunctionSerializerTest, MakeVariableFromConfigKeyReturnsNullForUnknownPrefix)
{
    EXPECT_EQ(makeVariableFromConfigKey("unknown:foo", nullptr), nullptr);
}

/**
 * @brief Round-trips sqrt(signal) + 5.0 through save/load into a fresh MathInputModel +
 * VariableRegistry.
 */
TEST(ValueFunctionSerializerTest, RoundTripsExpressionAndRebindsVariableToRegistry)
{
    MockEventBroker sourceBroker;
    VariableRegistry sourceRegistry(sourceBroker);
    MathInputModel sourceModel(sourceRegistry);

    auto* sourceVariable = sourceRegistry.acquire("signal:256:EngineRPM", [] {
        return std::make_unique<CanSignalVariable>(256, "EngineRPM", "MotorMsg");
    });
    dynamic_cast<CanSignalVariable*>(sourceVariable)->setValue(16.0);

    sourceModel.setVariableBindings(
        {VariableBinding{.symbol = 's', .typeIndex = 0, .variable = sourceVariable}});

    auto sqrtToken = std::make_unique<FunctionToken>(Function::Sqrt);
    sqrtToken->addChild(sourceModel.makeVariableToken("s"));

    auto root = std::make_unique<OperatorToken>(Operation::Add);
    root->addChild(std::move(sqrtToken));
    root->addChild(std::make_unique<ValueToken>(5.0));
    sourceModel.addNode(std::move(root), nullptr);

    const ValueFunction sourceFunction(sourceModel.root());
    // sqrt(16) + 5 == 9, sanity-checking the tree built above before it's serialized.
    ValueFunction sanityCheck(sourceModel.root());
    ASSERT_TRUE(sanityCheck.parse().success);
    EXPECT_DOUBLE_EQ(sanityCheck.evaluate(), 9.0);

    const auto path = tempFilePath();
    ASSERT_TRUE(ValueFunctionSerializer::save(path, sourceModel));

    MockEventBroker targetBroker;
    VariableRegistry targetRegistry(targetBroker);
    MathInputModel targetModel(targetRegistry);

    const auto loadedFunction = ValueFunctionSerializer::load(path, targetModel, targetRegistry);
    ASSERT_NE(loadedFunction, nullptr);
    ASSERT_TRUE(loadedFunction->isParsed());
    EXPECT_DOUBLE_EQ(loadedFunction->evaluate(), 5.0);
    auto* reacquired = targetRegistry.acquire("signal:256:EngineRPM", [] {
        return std::make_unique<CanSignalVariable>(256, "EngineRPM", "");
    });
    dynamic_cast<CanSignalVariable*>(reacquired)->setValue(16.0);

    EXPECT_DOUBLE_EQ(loadedFunction->evaluate(), 9.0);

    std::filesystem::remove(path);
}

TEST(ValueFunctionSerializerTest, LoadFromMissingFileReturnsNullptr)
{
    MockEventBroker broker;
    VariableRegistry registry(broker);
    MathInputModel model(registry);

    const auto loaded = ValueFunctionSerializer::load(
        "/tmp/this_file_should_not_exist_123456789.json", model, registry);
    EXPECT_EQ(loaded, nullptr);
}

TEST(ValueFunctionSerializerTest, DecodeReturnsNullptrForUntouchedSignal)
{
    constexpr auto* raw = R"({"bindings": [], "tree": null})";
    const auto json = nlohmann::json::parse(raw);

    MockEventBroker broker;
    VariableRegistry registry(broker);
    MathInputModel model(registry);

    const auto fn = ValueFunctionSerializer::decode(json, model, registry);
    EXPECT_EQ(fn, nullptr);
    EXPECT_EQ(model.root(), nullptr);
}
