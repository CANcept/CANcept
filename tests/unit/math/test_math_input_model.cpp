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

#include <QSignalSpy>
#include <memory>

#include "math/service/variable_registry.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "tests/helpers/mock_event_broker.hpp"

using namespace Math;
using namespace TestHelpers;

class MathInputModelTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        broker = std::make_unique<MockEventBroker>();
        registry = std::make_unique<VariableRegistry>(*broker);
        model = std::make_unique<MathInputModel>(*registry);
    }

    std::unique_ptr<MockEventBroker> broker;
    std::unique_ptr<VariableRegistry> registry;
    std::unique_ptr<MathInputModel> model;
};

// Root management
TEST_F(MathInputModelTest, RootIsNullInitially)
{
    EXPECT_EQ(model->root(), nullptr);
}

TEST_F(MathInputModelTest, AddNodeSetsRoot)
{
    model->addNode(std::make_unique<ValueToken>(5.0), nullptr);
    ASSERT_NE(model->root(), nullptr);
}

TEST_F(MathInputModelTest, RemoveRootClearsTree)
{
    model->addNode(std::make_unique<ValueToken>(5.0), nullptr);
    const auto* root = model->root();
    model->removeNode(root);
    EXPECT_EQ(model->root(), nullptr);
}

TEST_F(MathInputModelTest, RemoveNullDoesNotCrash)
{
    EXPECT_NO_THROW(model->removeNode(nullptr));
}

TEST_F(MathInputModelTest, AddNodeEmitsChanged)
{
    const QSignalSpy spy(model.get(), &MathInputModel::changed);
    model->addNode(std::make_unique<ValueToken>(1.0), nullptr);
    EXPECT_GE(spy.count(), 1);
}

// Child node management
TEST_F(MathInputModelTest, AddChildToInternalNode)
{
    auto op = std::make_unique<OperatorToken>(Operation::Add);
    auto* rawOp = op.get();
    model->addNode(std::move(op), nullptr);

    model->addNode(std::make_unique<ValueToken>(1.0), rawOp, 0);
    model->addNode(std::make_unique<ValueToken>(2.0), rawOp, 1);

    EXPECT_NE(rawOp->children()[0], nullptr);
    EXPECT_NE(rawOp->children()[1], nullptr);
}

TEST_F(MathInputModelTest, RemoveChildFromTree)
{
    auto op = std::make_unique<OperatorToken>(Operation::Add);
    auto* rawOp = op.get();
    model->addNode(std::move(op), nullptr);
    model->addNode(std::make_unique<ValueToken>(1.0), rawOp, 0);
    model->addNode(std::make_unique<ValueToken>(2.0), rawOp, 1);

    const auto* child = rawOp->children()[0].get();
    model->removeNode(child);
    EXPECT_EQ(rawOp->children()[0], nullptr);
}

// Variable bindings
TEST_F(MathInputModelTest, BindingsEmptyInitially)
{
    EXPECT_TRUE(model->variableBindings().empty());
}

TEST_F(MathInputModelTest, SetVariableBindingsStoresBindings)
{
    auto* var = registry->acquire("time:seconds",
                                  [] { return std::make_unique<TimeVariable>(TimeUnit::Seconds); });
    std::vector<VariableBinding> bindings;
    bindings.push_back({.symbol = 't', .typeIndex = 0, .variable = var});
    model->setVariableBindings(std::move(bindings));

    ASSERT_EQ(model->variableBindings().size(), 1);
    EXPECT_EQ(model->variableBindings()[0].symbol, 't');
    EXPECT_EQ(model->variableBindings()[0].variable, var);
}

TEST_F(MathInputModelTest, SetVariableBindingsReleasesOldOnes)
{
    auto* var = registry->acquire("time:seconds",
                                  [] { return std::make_unique<TimeVariable>(TimeUnit::Seconds); });
    registry->acquire("time:seconds",
                      [] { return std::make_unique<TimeVariable>(TimeUnit::Seconds); });

    std::vector<VariableBinding> bindings;
    bindings.push_back({.symbol = 't', .typeIndex = 0, .variable = var});
    model->setVariableBindings(std::move(bindings));
    model->setVariableBindings({});
    EXPECT_TRUE(model->variableBindings().empty());
}

TEST_F(MathInputModelTest, MakeVariableTokenForBoundSymbol)
{
    auto* var = registry->acquire("time:seconds",
                                  [] { return std::make_unique<TimeVariable>(TimeUnit::Seconds); });
    std::vector<VariableBinding> bindings;
    bindings.push_back({.symbol = 't', .typeIndex = 0, .variable = var});
    model->setVariableBindings(std::move(bindings));

    const auto token = model->makeVariableToken("t");
    ASSERT_NE(token, nullptr);
    EXPECT_EQ(token->toExpression(), "t");
}

TEST_F(MathInputModelTest, MakeVariableTokenReturnsNullForUnknownSymbol)
{
    const auto token = model->makeVariableToken("z");
    EXPECT_EQ(token, nullptr);
}

TEST_F(MathInputModelTest, MakeVariableTokenReturnsNullForMultiChar)
{
    const auto token = model->makeVariableToken("abc");
    EXPECT_EQ(token, nullptr);
}

// Slot navigation
TEST_F(MathInputModelTest, FreshModelHasNoActiveSlot)
{
    EXPECT_FALSE(model->isSlotActive());
}

TEST_F(MathInputModelTest, SelectNextEmptySlotOnEmptyTreeActivatesRootSlot)
{
    model->selectNextEmptySlot();
    EXPECT_TRUE(model->isSlotActive());
}

TEST_F(MathInputModelTest, CompleteTreeHasNoActiveSlot)
{
    model->addNode(std::make_unique<ValueToken>(1.0), nullptr);
    EXPECT_FALSE(model->isSlotActive());
}

TEST_F(MathInputModelTest, InternalNodeWithEmptySlotsIsIncomplete)
{
    model->addNode(std::make_unique<OperatorToken>(Operation::Add), nullptr);
    EXPECT_FALSE(model->isComplete());
}

TEST_F(MathInputModelTest, FilledInternalNodeIsComplete)
{
    auto op = std::make_unique<OperatorToken>(Operation::Add);
    auto* rawOp = op.get();
    model->addNode(std::move(op), nullptr);
    model->addNode(std::make_unique<ValueToken>(1.0), rawOp, 0);
    model->addNode(std::make_unique<ValueToken>(2.0), rawOp, 1);
    EXPECT_TRUE(model->isComplete());
}

TEST_F(MathInputModelTest, SelectNextEmptySlotFindsIncompleteChild)
{
    auto op = std::make_unique<OperatorToken>(Operation::Add);
    auto* rawOp = op.get();
    model->addNode(std::move(op), nullptr);
    model->addNode(std::make_unique<ValueToken>(1.0), rawOp, 0);

    // Slot 1 is still empty — selectNextEmptySlot should find it
    model->selectNextEmptySlot();
    EXPECT_TRUE(model->isSlotActive());
    const auto slot = model->activeSlot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->parent, rawOp);
    EXPECT_EQ(slot->childIndex, 1);
}

// Type buffer
TEST_F(MathInputModelTest, TypeBufferEmptyInitially)
{
    EXPECT_TRUE(model->typeBuffer().isEmpty());
}

TEST_F(MathInputModelTest, AppendToTypeBuffer)
{
    model->appendToTypeBuffer("a");
    model->appendToTypeBuffer("b");
    EXPECT_EQ(model->typeBuffer(), "ab");
}

TEST_F(MathInputModelTest, ChopTypeBuffer)
{
    model->appendToTypeBuffer("abc");
    model->chopTypeBuffer();
    EXPECT_EQ(model->typeBuffer(), "ab");
}

TEST_F(MathInputModelTest, ClearEditorStateResetsBuffer)
{
    model->appendToTypeBuffer("test");
    model->clearEditorState();
    EXPECT_TRUE(model->typeBuffer().isEmpty());
    EXPECT_FALSE(model->isSlotActive());
}

TEST_F(MathInputModelTest, CommitTypeBufferInsertsNumericValue)
{
    model->selectNextEmptySlot();
    ASSERT_TRUE(model->isSlotActive());

    model->appendToTypeBuffer("42");
    model->commitTypeBuffer();

    ASSERT_NE(model->root(), nullptr);
    EXPECT_TRUE(model->typeBuffer().isEmpty());
}

TEST_F(MathInputModelTest, CommitTypeBufferInsertsVariable)
{
    auto* var = registry->acquire("time:seconds",
                                  [] { return std::make_unique<TimeVariable>(TimeUnit::Seconds); });
    std::vector<VariableBinding> bindings;
    bindings.push_back({.symbol = 't', .typeIndex = 0, .variable = var});
    model->setVariableBindings(std::move(bindings));

    model->selectNextEmptySlot();
    ASSERT_TRUE(model->isSlotActive());
    model->appendToTypeBuffer("t");
    model->commitTypeBuffer();

    ASSERT_NE(model->root(), nullptr);
    EXPECT_EQ(model->root()->toExpression(), "t");
}

TEST_F(MathInputModelTest, CommitEmptyBufferDoesNothing)
{
    model->selectNextEmptySlot();
    ASSERT_TRUE(model->isSlotActive());
    model->commitTypeBuffer();
    EXPECT_EQ(model->root(), nullptr);
}

// Editor state signals
TEST_F(MathInputModelTest, ActivateSlotEmitsEditorStateChanged)
{
    auto op = std::make_unique<OperatorToken>(Operation::Add);
    const auto* rawOp = op.get();
    model->addNode(std::move(op), nullptr);

    QSignalSpy spy(model.get(), &MathInputModel::editorStateChanged);
    model->activateSlot(rawOp, 0);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(MathInputModelTest, AppendToTypeBufferEmitsEditorStateChanged)
{
    QSignalSpy spy(model.get(), &MathInputModel::editorStateChanged);
    model->appendToTypeBuffer("x");
    EXPECT_GE(spy.count(), 1);
}
