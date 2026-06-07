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

#include <clocale>
#include <cmath>
#include <memory>
#include <vector>

#include "math/service/value_function.hpp"
#include "math/service/variable_registry.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"
#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "tests/helpers/mock_event_broker.hpp"
#include "tests/helpers/token_builders.hpp"

using namespace Math;
using namespace TestHelpers;

class MathEvalIntegration : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        std::setlocale(LC_NUMERIC, "C");

        broker = std::make_unique<TestHelpers::MockEventBroker>();
        registry = std::make_unique<VariableRegistry>(*broker);
        model = std::make_unique<MathInputModel>(*registry);
    }

    auto bindTime(const char symbol, const TimeUnit unit = TimeUnit::Seconds) -> IVariable*
    {
        const std::string key = "time:" + TimeVariable::unitSuffix(unit);
        auto* var = registry->acquire(key, [unit] { return std::make_unique<TimeVariable>(unit); });
        addBinding(symbol, var);
        return var;
    }

    auto bindSignal(const char symbol, const uint32_t msgId, const std::string& sigName,
                    const std::string& msgName) -> CanSignalVariable*
    {
        const std::string key = "signal:" + std::to_string(msgId) + ":" + sigName;
        auto* var = registry->acquire(key, [msgId, &sigName, &msgName] {
            return std::make_unique<CanSignalVariable>(msgId, sigName, msgName);
        });
        auto* canVar = dynamic_cast<CanSignalVariable*>(var);
        addBinding(symbol, var);
        return canVar;
    }

    void addBinding(const char symbol, IVariable* var)
    {
        m_pendingBindings.push_back({.symbol = symbol, .typeIndex = 0, .variable = var});
    }

    void commitBindings()
    {
        model->setVariableBindings(std::move(m_pendingBindings));
        m_pendingBindings.clear();
    }

    // Build tree via model API (insertToken flow) and evaluate through ValueFunction
    auto buildAndEvaluate() -> double
    {
        EXPECT_TRUE(model->isComplete());
        ValueFunction vf(model->root());
        const auto result = vf.parse();
        EXPECT_TRUE(result.success) << "Parse failed: " << result.error;
        registry->updateAll();
        return vf.evaluate();
    }

    std::unique_ptr<TestHelpers::MockEventBroker> broker;
    std::unique_ptr<VariableRegistry> registry;
    std::unique_ptr<MathInputModel> model;
    std::vector<VariableBinding> m_pendingBindings;
};

TEST_F(MathEvalIntegration, NestedArithmeticWithMultipleSignalsMutated)
{
    auto* sigA = bindSignal('a', 0x100, "RPM", "MotorMsg");
    auto* sigB = bindSignal('b', 0x100, "Torque", "MotorMsg");
    commitBindings();

    // Build tree manually: div( mul( add(a, b), sub(a, b) ), 2.0 )
    auto varA1 = model->makeVariableToken("a");
    auto varB1 = model->makeVariableToken("b");
    auto varA2 = model->makeVariableToken("a");
    auto varB2 = model->makeVariableToken("b");

    auto sum = makeAdd(std::move(varA1), std::move(varB1));
    auto diff = makeSub(std::move(varA2), std::move(varB2));
    auto product = makeMul(std::move(sum), std::move(diff));
    auto tree = makeDiv(std::move(product), std::make_unique<ValueToken>(2.0));

    model->addNode(std::move(tree), nullptr);

    // Set initial values: a=5, b=3
    sigA->setValue(5.0);
    sigB->setValue(3.0);

    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);
    EXPECT_DOUBLE_EQ(vf.evaluate(), 8.0);  // (5+3)*(5-3)/2 = 8

    // Mutate and re-evaluate without re-parsing
    sigA->setValue(10.0);
    sigB->setValue(4.0);
    EXPECT_DOUBLE_EQ(vf.evaluate(), 42.0);  // (10+4)*(10-4)/2 = 42
}

TEST_F(MathEvalIntegration, TrigonometricExpressionWithTimeVariable)
{
    auto* timeVar = bindTime('t');
    commitBindings();

    // Build: add( mul( sin(t), cos(t) ), abs( sub( sin(t), 0.5 ) ) )
    auto varT1 = model->makeVariableToken("t");
    auto varT2 = model->makeVariableToken("t");
    auto varT3 = model->makeVariableToken("t");

    auto sinT1 = makeFunc(Function::Sin, std::move(varT1));
    auto cosT = makeFunc(Function::Cos, std::move(varT2));
    auto sinCos = makeMul(std::move(sinT1), std::move(cosT));

    auto sinT2 = makeFunc(Function::Sin, std::move(varT3));
    auto diff = makeSub(std::move(sinT2), std::make_unique<ValueToken>(0.5));
    auto absDiff = makeFunc(Function::Abs, std::move(diff));

    auto tree = makeAdd(std::move(sinCos), std::move(absDiff));
    model->addNode(std::move(tree), nullptr);

    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);

    // Set t = 0: sin(0)*cos(0) + abs(sin(0) - 0.5) = 0*1 + 0.5 = 0.5
    *timeVar->ptr() = 0.0;
    EXPECT_NEAR(vf.evaluate(), 0.5, 1e-10);

    // Set t = pi/2: sin(pi/2)*cos(pi/2) + abs(sin(pi/2) - 0.5) = 1*0 + 0.5 = 0.5
    *timeVar->ptr() = M_PI / 2.0;
    EXPECT_NEAR(vf.evaluate(), 0.5, 1e-10);

    // Set t = pi/6: sin(pi/6)*cos(pi/6) + abs(sin(pi/6) - 0.5)
    //             = 0.5 * (sqrt(3)/2) + abs(0.5 - 0.5)
    //             = sqrt(3)/4 + 0 = sqrt(3)/4
    *timeVar->ptr() = M_PI / 6.0;
    EXPECT_NEAR(vf.evaluate(), std::sqrt(3.0) / 4.0, 1e-10);
}

TEST_F(MathEvalIntegration, InteractiveTreeBuildingWithCommitTypeBuffer)
{
    auto* sigRpm = bindSignal('r', 0x200, "RPM", "Engine");
    commitBindings();

    // Insert an Add operator as root via insertToken
    model->selectNextEmptySlot();
    ASSERT_TRUE(model->isSlotActive());
    model->insertToken(std::make_unique<OperatorToken>(Operation::Mul));

    // Now the tree is: mul(□, □) — slot 0 should be active
    ASSERT_TRUE(model->isSlotActive());
    ASSERT_FALSE(model->isComplete());

    // Type "r" into slot 0 (variable)
    model->appendToTypeBuffer("r");
    model->commitTypeBuffer();

    // Slot 1 should now be active
    ASSERT_TRUE(model->isSlotActive());
    ASSERT_FALSE(model->isComplete());

    // Type "100" into slot 1 (numeric constant)
    model->appendToTypeBuffer("100");
    model->commitTypeBuffer();

    // Tree is now complete: mul(r, 100)
    ASSERT_TRUE(model->isComplete());

    sigRpm->setValue(25.0);

    ValueFunction vf(model->root());
    std::cout << "Expression: " << vf.expression() << std::endl;
    ASSERT_TRUE(vf.parse().success) << "Parse error: " << vf.lastResult().error;
    EXPECT_DOUBLE_EQ(vf.evaluate(), 2500.0);  // 25 * 100

    // Mutate RPM and re-evaluate
    sigRpm->setValue(60.0);
    EXPECT_DOUBLE_EQ(vf.evaluate(), 6000.0);  // 60 * 100
}

TEST_F(MathEvalIntegration, SharedVariableAcrossMultipleModelsRefCounting)
{
    auto model2 = std::make_unique<MathInputModel>(*registry);

    // Both models acquire the same signal variable
    const std::string key = "signal:300:Speed";
    auto* var1 = registry->acquire(
        key, [] { return std::make_unique<CanSignalVariable>(0x300, "Speed", "VehicleMsg"); });
    auto* var2 = registry->acquire(
        key, [] { return std::make_unique<CanSignalVariable>(0x300, "Speed", "VehicleMsg"); });

    // Same variable instance (deduplicated)
    EXPECT_EQ(var1, var2);

    // Bind to model1 as 's', model2 as 'v'
    model->setVariableBindings({{.symbol = 's', .typeIndex = 0, .variable = var1}});
    model2->setVariableBindings({{.symbol = 'v', .typeIndex = 0, .variable = var2}});

    // Build simple trees: model1 = s * 2, model2 = v + 10
    auto varS = model->makeVariableToken("s");
    auto tree1 = makeMul(std::move(varS), std::make_unique<ValueToken>(2.0));
    model->addNode(std::move(tree1), nullptr);

    auto varV = model2->makeVariableToken("v");
    auto tree2 = makeAdd(std::move(varV), std::make_unique<ValueToken>(10.0));
    model2->addNode(std::move(tree2), nullptr);

    auto* canVar = dynamic_cast<CanSignalVariable*>(var1);
    canVar->setValue(50.0);

    ValueFunction vf1(model->root());
    ValueFunction vf2(model2->root());
    ASSERT_TRUE(vf1.parse().success);
    ASSERT_TRUE(vf2.parse().success);

    EXPECT_DOUBLE_EQ(vf1.evaluate(), 100.0);  // 50 * 2
    EXPECT_DOUBLE_EQ(vf2.evaluate(), 60.0);   // 50 + 10

    // Destroy model2 — should release its ref but variable stays alive for model1
    model2.reset();

    canVar->setValue(75.0);
    EXPECT_DOUBLE_EQ(vf1.evaluate(), 150.0);  // 75 * 2 — still works
}

TEST_F(MathEvalIntegration, DeepNestingAllOperatorsAndFunctions)
{
    auto* sigA = bindSignal('a', 0x400, "Temp", "SensorMsg");
    auto* sigB = bindSignal('b', 0x400, "Pressure", "SensorMsg");
    commitBindings();

    // Build: sqrt( abs( add( mul( sin(a), cos(b) ), log( div(a, b) ) ) ) )
    auto varA1 = model->makeVariableToken("a");
    auto varB1 = model->makeVariableToken("b");
    auto varA2 = model->makeVariableToken("a");
    auto varB2 = model->makeVariableToken("b");

    auto sinA = makeFunc(Function::Sin, std::move(varA1));
    auto cosB = makeFunc(Function::Cos, std::move(varB1));
    auto sinCos = makeMul(std::move(sinA), std::move(cosB));

    auto aOverB = makeDiv(std::move(varA2), std::move(varB2));
    auto logAB = makeFunc(Function::Log, std::move(aOverB));

    auto sum = makeAdd(std::move(sinCos), std::move(logAB));
    auto absSum = makeFunc(Function::Abs, std::move(sum));
    auto tree = makeFunc(Function::Sqrt, std::move(absSum));

    model->addNode(std::move(tree), nullptr);

    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);

    // a=1, b=1: sqrt( abs( sin(1)*cos(1) + log(1) ) )
    //         = sqrt( abs( 0.8414*0.5403 + 0 ) )
    //         = sqrt( 0.45465 ) ≈ 0.6743
    sigA->setValue(1.0);
    sigB->setValue(1.0);
    const double sinOne = std::sin(1.0);
    const double cosOne = std::cos(1.0);
    const double expected1 = std::sqrt(std::abs(sinOne * cosOne + std::log(1.0)));
    EXPECT_NEAR(vf.evaluate(), expected1, 1e-10);

    // a=e, b=1: sqrt( abs( sin(e)*cos(1) + log(e) ) )
    //         = sqrt( abs( sin(e)*cos(1) + 1 ) )
    sigA->setValue(M_E);
    sigB->setValue(1.0);
    const double expected2 = std::sqrt(std::abs(std::sin(M_E) * std::cos(1.0) + std::log(M_E)));
    EXPECT_NEAR(vf.evaluate(), expected2, 1e-10);

    // a=2, b=0.5: sqrt( abs( sin(2)*cos(0.5) + log(4) ) )
    sigA->setValue(2.0);
    sigB->setValue(0.5);
    const double expected3 = std::sqrt(std::abs(std::sin(2.0) * std::cos(0.5) + std::log(4.0)));
    EXPECT_NEAR(vf.evaluate(), expected3, 1e-10);
}

TEST_F(MathEvalIntegration, VariableRebindingChangesEvaluation)
{
    auto* timeVar = bindTime('t');
    commitBindings();

    // Build: mul(t, 2)
    auto varT = model->makeVariableToken("t");
    auto tree = makeMul(std::move(varT), std::make_unique<ValueToken>(2.0));
    model->addNode(std::move(tree), nullptr);

    *timeVar->ptr() = 5.0;

    ValueFunction vf1(model->root());
    ASSERT_TRUE(vf1.parse().success);
    EXPECT_DOUBLE_EQ(vf1.evaluate(), 10.0);  // 5 * 2

    // Now rebind 't' to a CAN signal and build a new tree
    auto* sigVar = registry->acquire("signal:500:Voltage", [] {
        return std::make_unique<CanSignalVariable>(0x500, "Voltage", "BMS");
    });

    // setVariableBindings releases the old time ref
    model->setVariableBindings({{.symbol = 't', .typeIndex = 0, .variable = sigVar}});

    // Remove old tree and build a new one with the rebound variable
    model->removeNode(model->root());
    auto varT2 = model->makeVariableToken("t");
    auto tree2 = makeMul(std::move(varT2), std::make_unique<ValueToken>(2.0));
    model->addNode(std::move(tree2), nullptr);

    auto* canSig = dynamic_cast<CanSignalVariable*>(sigVar);
    canSig->setValue(12.0);

    ValueFunction vf2(model->root());
    ASSERT_TRUE(vf2.parse().success);
    EXPECT_DOUBLE_EQ(vf2.evaluate(), 24.0);  // 12 * 2
}

TEST_F(MathEvalIntegration, InteractiveNestedFunctionBuild)
{
    auto* sigA = bindSignal('a', 0x600, "Angle", "Steering");
    commitBindings();

    // Step 1: Insert add as root
    model->selectNextEmptySlot();
    model->insertToken(std::make_unique<OperatorToken>(Operation::Add));

    // Step 2: Insert sin into slot 0 of add
    ASSERT_TRUE(model->isSlotActive());
    model->insertToken(std::make_unique<FunctionToken>(Function::Sin));

    // Step 3: Slot should now be inside sin — type variable 'a'
    ASSERT_TRUE(model->isSlotActive());
    model->appendToTypeBuffer("a");
    model->commitTypeBuffer();

    // Step 4: Slot should now be add's slot 1 — type constant
    ASSERT_TRUE(model->isSlotActive());
    model->appendToTypeBuffer("3.14");
    model->commitTypeBuffer();

    // Tree: add(sin(a), 3.14)
    ASSERT_TRUE(model->isComplete());

    *sigA->ptr() = 0.0;
    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);
    EXPECT_NEAR(vf.evaluate(), 3.14, 1e-10);  // sin(0) + 3.14

    *sigA->ptr() = M_PI / 2.0;
    EXPECT_NEAR(vf.evaluate(), 1.0 + 3.14, 1e-10);  // sin(pi/2) + 3

    *sigA->ptr() = M_PI;
    EXPECT_NEAR(vf.evaluate(), std::sin(M_PI) + 3.14, 1e-10);
}

TEST_F(MathEvalIntegration, UpdateAllRefreshesTimeVariablesBeforeEval)
{
    auto* timeVar = bindTime('t');
    commitBindings();

    // Build: add(t, 1.0)
    auto varT = model->makeVariableToken("t");
    auto tree = makeAdd(std::move(varT), std::make_unique<ValueToken>(1.0));
    model->addNode(std::move(tree), nullptr);

    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);

    // First evaluation — time should be very small (just started)
    registry->updateAll();
    const double firstEval = vf.evaluate();
    EXPECT_GE(firstEval, 1.0);  // At least the constant 1.0

    // Manually set time to a known value
    *timeVar->ptr() = 10.0;
    const double secondEval = vf.evaluate();
    EXPECT_DOUBLE_EQ(secondEval, 11.0);  // 10 + 1

    // After updateAll, time should be overwritten with actual elapsed time
    registry->updateAll();
    const double thirdEval = vf.evaluate();
    EXPECT_GE(thirdEval, 1.0);
    EXPECT_NE(thirdEval, 11.0);  // Should no longer be our manually set value
}

TEST_F(MathEvalIntegration, MixedSignalsAndTimeInSingleExpression)
{
    auto* rpmVar = bindSignal('r', 0x100, "RPM", "MotorMsg");
    auto* torqueVar = bindSignal('q', 0x100, "Torque", "MotorMsg");
    auto* timeVar = bindTime('t');
    commitBindings();

    // Build: add( mul( div(r, 1000), sin(t) ), abs(q) )
    auto varR = model->makeVariableToken("r");
    auto varT = model->makeVariableToken("t");
    auto varQ = model->makeVariableToken("q");

    auto rpmScaled = makeDiv(std::move(varR), std::make_unique<ValueToken>(1000.0));
    auto sinT = makeFunc(Function::Sin, std::move(varT));
    auto rpmSin = makeMul(std::move(rpmScaled), std::move(sinT));
    auto absQ = makeFunc(Function::Abs, std::move(varQ));
    auto tree = makeAdd(std::move(rpmSin), std::move(absQ));

    model->addNode(std::move(tree), nullptr);

    ValueFunction vf(model->root());
    ASSERT_TRUE(vf.parse().success);

    // Snapshot 1: rpm=3000, torque=-150, t=0
    rpmVar->setValue(3000.0);
    torqueVar->setValue(-150.0);
    *timeVar->ptr() = 0.0;
    // (3000/1000)*sin(0) + abs(-150) = 0 + 150 = 150
    EXPECT_NEAR(vf.evaluate(), 150.0, 1e-10);

    // Snapshot 2: rpm=6000, torque=200, t=pi/2
    rpmVar->setValue(6000.0);
    torqueVar->setValue(200.0);
    *timeVar->ptr() = M_PI / 2.0;
    // (6000/1000)*sin(pi/2) + abs(200) = 6*1 + 200 = 206
    EXPECT_NEAR(vf.evaluate(), 206.0, 1e-10);

    // Snapshot 3: rpm=1000, torque=-50, t=pi
    rpmVar->setValue(1000.0);
    torqueVar->setValue(-50.0);
    *timeVar->ptr() = M_PI;
    // (1000/1000)*sin(pi) + abs(-50) = ~0 + 50 = 50
    EXPECT_NEAR(vf.evaluate(), 50.0, 1e-10);
}